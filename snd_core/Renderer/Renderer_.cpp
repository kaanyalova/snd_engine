#include "Renderer.hpp"

#include <SDL3/SDL_video.h>
#include <imgui_impl_vulkan.h>
#include <ktx.h>
#include <ktxvulkan.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <print>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vk_mem_alloc_enums.hpp>
#include <vk_mem_alloc_handles.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vk_mem_alloc_structs.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "../Image/KtxImage.hpp"
#include "../RendererInfoGui.hpp"
#include "../UniformBuffer.hpp"
#include "../Utils/FileUtils.hpp"
#include "../Utils/ImGuiUtils.hpp"
#include "../Utils/StringUtils.hpp"
#include "../Vertex.hpp"
#include "./RendererStructs/BufferInfo.hpp"
#include "./RendererStructs/ImageInfo.hpp"
#include "./RendererStructs/ImageTransitionInfo.hpp"
#include "./RendererStructs/ImageViewInfo.hpp"

Renderer::Renderer(Window& window, RendererSettings settings)
    : m_required_extensions(std::move(settings.required_extensions))
    , m_validation_layers(std::move(settings.validation_layers))
    , m_enable_validation(settings.enable_validation)
    , m_validation_log_level(settings.validation_log_level)
    , m_validation_message_types(settings.validation_message_types)
    , m_gpu_preference(settings.gpu_preference)
    , m_window(window) {
}

auto Renderer::process() -> void {
    update_clocks_before_draw();
    draw_frame();
    update_clocks_after_draw();

    RendererInfoGui::set_frametime(m_last_frame_time);
}

auto Renderer::init_vulkan() -> void {
    create_instance();
    if (m_enable_validation) {
        setup_debug_messenger();
    }
    create_surface();
    pick_physical_device();
    get_physical_device_properties();
    create_logical_device();
    create_memory_allocator();

    create_swap_chain();
    create_swapchain_image_views();

    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_command_pool();

    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    load_assets();
    create_image_sampler();

    create_descriptor_sets();
    create_command_buffers();
    create_sync_objects();
}

auto Renderer::load_assets() -> void {
    run_single_time_commands([&](vk::raii::CommandBuffer& command_buffer) {
        load_texture(command_buffer, "./textures/test.ktx");
    });
}

auto Renderer::create_instance() -> void {
    auto constexpr application_info = vk::ApplicationInfo {
        .pApplicationName = "vulkan",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };
    std::vector<const char*> sdl_extensions = m_window.get_required_vulkan_extensions();

    for (const char* extension : sdl_extensions) {
        std::println("sdl required extension: {}", extension);
    }

    m_required_extensions.insert(
        m_required_extensions.end(), sdl_extensions.begin(), sdl_extensions.end()
    );

    // m_required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);

    if (m_enable_validation) {
        m_required_extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    bool extensions_supported = are_required_extensions_supported_by_instance();

    if (!extensions_supported) {
        throw std::runtime_error("some sdl3 extensions are not supported for vulkan");
    }

    std::vector<const char*> validation_layers = get_validation_layers();

    auto create_info = vk::InstanceCreateInfo {
        //.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
        .ppEnabledLayerNames = validation_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(m_required_extensions.size()),
        .ppEnabledExtensionNames = m_required_extensions.data(),

    };

    m_context.instance = vk::raii::Instance(m_context.vulkan_context, create_info);
}

/**
 * @brief check if all the extensions in m_required_extensions are supported by the instance
 *
 * @return true if all extensions are supported, false otherwise
 */
auto Renderer::are_required_extensions_supported_by_instance() -> bool {
    std::vector<vk::ExtensionProperties> extension_properties =
        m_context.vulkan_context.enumerateInstanceExtensionProperties();

    return std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
        return std::ranges::any_of(
            extension_properties, [&](const vk::ExtensionProperties& available_extension) -> bool {
                return std::strcmp(required_extension, available_extension.extensionName) == 0;
            }
        );
    });
}

/**
 * @brief Get the validation layers to enable
 *
 * @return If validation is enabled in the config and all the validation layers are supported
 * return the validation layers, otherwise return an empty vector
 */
auto Renderer::get_validation_layers() -> std::vector<const char*> {
    if (m_enable_validation) {
        if (!are_required_extensions_supported_by_instance()) {
            throw std::runtime_error("some validation layers are not supported");
        }
        return std::move(m_validation_layers);
    }
    return {};
}

/** */
auto Renderer::find_queue_families(const vk::raii::PhysicalDevice& device) -> FamilyIndices {
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();

    const auto graphics_queue_family_property = std::ranges::find_if(
        queue_family_properties, [](const vk::QueueFamilyProperties& qfp) -> bool {
            return static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    if (graphics_queue_family_property == queue_family_properties.end()) {
        throw std::runtime_error("failed to find a graphics queue family");
    }

    auto graphics_family_index = static_cast<uint32_t>(
        std::distance(queue_family_properties.begin(), graphics_queue_family_property)
    );

    // check if the graphics family also supports presentation
    bool also_supports_presentation =
        device.getSurfaceSupportKHR(graphics_family_index, *m_surface);

    uint32_t presentation_family_index = 0;

    if (also_supports_presentation) {
        presentation_family_index = graphics_family_index;
    } else {
        auto iota = std::views::iota(0u, static_cast<uint32_t>(queue_family_properties.size()));
        auto presentation_family_it = std::ranges::find_if(iota, [&](uint32_t index) -> bool {
            return device.getSurfaceSupportKHR(index, *m_surface);
        });

        if (presentation_family_it == iota.end()) {
            throw std::runtime_error("failed to find a presentation queue family");
        }

        presentation_family_index = *presentation_family_it;
    }

    return FamilyIndices {
        .graphics_family = graphics_family_index,
        .presentation_family = presentation_family_index,
    };
}

auto Renderer::setup_debug_messenger() -> void {
    auto messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT {
        .messageSeverity = m_validation_log_level,
        .messageType = m_validation_message_types,
        .pfnUserCallback = &debug_callback,
    };

    m_debug_messenger = m_context.instance.createDebugUtilsMessengerEXT(messenger_create_info);
}

/**
 * @brief Pick the PhysicalDevice according to the m_gpu_preference, fallback to any GPU if
 * the preference is not found
 */
auto Renderer::pick_physical_device() -> void {
    std::vector<vk::raii::PhysicalDevice> physical_devices =
        m_context.instance.enumeratePhysicalDevices();

    if (m_gpu_preference == GpuPreference::Discrete) {
        const auto discrete_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                       is_device_suitable(device);
            });

        if (discrete_gpu_it != physical_devices.end()) {
            m_context.physical_device = *discrete_gpu_it;
            return;
        }
    } else if (m_gpu_preference == GpuPreference::Integrated) {
        const auto integrated_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType ==
                           vk::PhysicalDeviceType::eIntegratedGpu &&
                       is_device_suitable(device);
            });

        if (integrated_gpu_it != physical_devices.end()) {
            m_context.physical_device = *integrated_gpu_it;
            return;
        }
    }

    // cannot find the preferred gpu, fallback to any one that works
    auto fallback_it =
        std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
            return is_device_suitable(device);
        });

    if (fallback_it != physical_devices.end()) {
        m_context.physical_device = *fallback_it;
        return;
    }

    // no gpus are suitable
    throw std::runtime_error("failed to find a suitable GPU");
}

auto Renderer::get_physical_device_properties() -> void {
    vk::PhysicalDeviceProperties properties = m_context.physical_device.getProperties();
    m_max_sampler_anisotropy = properties.limits.maxSamplerAnisotropy;
}

auto Renderer::create_logical_device() -> void {
    m_family_indices = find_queue_families(m_context.physical_device);
    float queue_priority = 1.0f;

    auto device_queue_create_info = vk::DeviceQueueCreateInfo {
        .queueFamilyIndex = m_family_indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    using DeviceFeaturesChain = vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>;

    auto feature_chain = DeviceFeaturesChain {
        vk::PhysicalDeviceFeatures2 {
            .features =
                vk::PhysicalDeviceFeatures {
                    .samplerAnisotropy = vk::True,
                },
        },
        vk::PhysicalDeviceVulkan11Features {.shaderDrawParameters = vk::True},
        vk::PhysicalDeviceVulkan13Features {
            .synchronization2 = vk::True, .dynamicRendering = vk::True
        },
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {.extendedDynamicState = vk::True},
    };

    std::vector<const char*> device_extensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRSwapchainExtensionName,
    };

    auto device_create_info = vk::DeviceCreateInfo {
        .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
    };

    m_context.device = vk::raii::Device(m_context.physical_device, device_create_info);
    m_graphics_queue = vk::raii::Queue(m_context.device, m_family_indices.graphics_family, 0);
    m_presentation_queue =
        vk::raii::Queue(m_context.device, m_family_indices.presentation_family, 0);
}

auto Renderer::create_memory_allocator() -> void {
    auto vma_allocator_create_info = vma::AllocatorCreateInfo {
        .physicalDevice = *m_context.physical_device,
        .vulkanApiVersion = vk::ApiVersion14,
    };

    m_allocator =
        vma::raii::createAllocator(m_context.instance, m_context.device, vma_allocator_create_info);
}

auto Renderer::create_surface() -> void {
    m_surface = m_window.create_surface(m_context.instance);
}

auto Renderer::debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
    void* _user_data
) -> vk::Bool32 {
    std::string type_string = vk::to_string(type);
    std::erase(type_string, '{');
    std::erase(type_string, '}');
    StringUtils::trim(type_string);

    std::println(
        "Validation Layer ({}) [{}] : {}",
        type_string,
        vk::to_string(severity),
        callback_data->pMessage
    );
    return vk::False;
}

/**
 * @brief Choose the swap surface format based on the available formats
 *
 * @param available_formats list of available formats by the PhysicalDevice
 * @return vk::SurfaceFormatKHR the chosen surface format
 */
auto Renderer::choose_swap_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats
) -> vk::SurfaceFormatKHR {
    auto available_format_it =
        std::ranges::find_if(available_formats, [](const vk::SurfaceFormatKHR& available_format) {
            return available_format.format == vk::Format::eB8G8R8A8Srgb &&
                   available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    if (available_format_it != available_formats.end()) {
        return *available_format_it;
    }

    // return the first one if the preferred one is not found
    return available_formats[0];
}

auto Renderer::choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_modes)
    -> vk::PresentModeKHR {
    auto mailbox_mode_it = std::ranges::find(available_modes, vk::PresentModeKHR::eMailbox);

    if (mailbox_mode_it != available_modes.end()) {
        return *mailbox_mode_it;
    }

    return vk::PresentModeKHR::eFifo;
}

auto Renderer::choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width;
    int height;
    bool result = SDL_GetWindowSizeInPixels(&m_window.sdl_window(), &width, &height);

    if (result != true) {
        throw std::runtime_error("failed to get window size for swap extent");
    }

    return vk::Extent2D {
        .width = std::clamp<uint32_t>(
            width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
        ),
        .height = std::clamp<uint32_t>(
            height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
        ),
    };
}

auto Renderer::create_swap_chain() -> void {
    auto surface_capabilities = m_context.physical_device.getSurfaceCapabilitiesKHR(m_surface);
    m_swap_chain_extent = choose_swap_extent(surface_capabilities);

    std::vector<vk::SurfaceFormatKHR> surface_formats =
        m_context.physical_device.getSurfaceFormatsKHR(m_surface);
    m_swap_chain_surface_format = choose_swap_surface_format(surface_formats);

    uint32_t min_image_count = std::max(3u, surface_capabilities.minImageCount);

    // clamp it between the surface's capabilities
    if (surface_capabilities.maxImageCount > 0) {
        min_image_count = std::clamp(
            min_image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount
        );
    }

    uint32_t image_count = surface_capabilities.minImageCount + 1;

    if (surface_capabilities.maxImageCount > 0 &&
        image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }

    vk::PresentModeKHR present_mode =

        choose_swap_present_mode(m_context.physical_device.getSurfacePresentModesKHR(*m_surface));

    auto swap_chain_create_info = vk::SwapchainCreateInfoKHR {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = *m_surface,
        .minImageCount = min_image_count,
        .imageFormat = m_swap_chain_surface_format.format,
        .imageColorSpace = m_swap_chain_surface_format.colorSpace,
        .imageExtent = m_swap_chain_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = true,
        .oldSwapchain = nullptr,
    };

    std::array<uint32_t, 2> family_indices = {
        m_family_indices.graphics_family,
        m_family_indices.presentation_family,
    };

    if (m_family_indices.graphics_family != m_family_indices.presentation_family) {
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        // todo check if this cast actually works
        swap_chain_create_info.pQueueFamilyIndices = family_indices.data();
    }

    m_swap_chain = vk::raii::SwapchainKHR(m_device, swap_chain_create_info);
    m_swap_chain_images = m_swap_chain.getImages();
}

auto Renderer::create_swapchain_image_views() -> void {
    m_swap_chain_image_views.clear();

    auto sub_resource_range = vk::ImageSubresourceRange {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    auto image_view_create_info = vk::ImageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = m_swap_chain_surface_format.format,
        .subresourceRange = sub_resource_range,
    };

    for (vk::Image image : m_swap_chain_images) {
        image_view_create_info.image = image;
        m_swap_chain_image_views.emplace_back(m_device, image_view_create_info);
    }
}

auto Renderer::create_image_view(const vk::raii::Image& image, ImageViewInfo info)
    -> vk::raii::ImageView {
    auto view_info = vk::ImageViewCreateInfo {
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = info.format,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = info.aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    return vk::raii::ImageView(m_device, view_info);
}

auto Renderer::find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties)
    -> uint32_t {
    vk::PhysicalDeviceMemoryProperties physical_device_memory_properties =
        m_physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
        const bool is_type = type_filter & (1 << i);
        const bool supports_properties =
            (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) ==
            properties;

        if (is_type && supports_properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

[[nodiscard]] auto Renderer::create_shader_module(const std::span<uint8_t>& code) const
    -> vk::raii::ShaderModule {
    auto shader_module_create_info = vk::ShaderModuleCreateInfo {
        .codeSize = code.size() * sizeof(uint8_t),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    auto shader_module = vk::raii::ShaderModule(m_device, shader_module_create_info);
    return shader_module;
}

auto Renderer::create_graphics_pipeline() -> void {
    // TODO: use relative paths here
    std::vector<uint8_t> shader_code = FileUtils::read_file("./src/shaders/shader.spv");
    vk::raii::ShaderModule shader_module = create_shader_module(shader_code);

    auto vert_shader_stage_info = vk::PipelineShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = *shader_module,
        .pName = "vertex_main",
    };

    auto frag_shader_stage_info = vk::PipelineShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = *shader_module,
        .pName = "fragment_main",
    };

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {
        vert_shader_stage_info,
        frag_shader_stage_info,
    };

    auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo {
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    auto viewport = vk::Viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_swap_chain_extent.width),
        .height = static_cast<float>(m_swap_chain_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    auto scissor = vk::Rect2D {
        .offset = vk::Offset2D {.x = 0, .y = 0},
        .extent = m_swap_chain_extent,
    };

    auto pipeline_viewport_state_info = vk::PipelineViewportStateCreateInfo {
        .viewportCount = 1,
        .scissorCount = 1,
    };

    auto pipeline_rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f,
    };

    auto pipeline_multi_sample_state_create_info = vk::PipelineMultisampleStateCreateInfo {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    auto pipeline_color_blend_attachment_state = vk::PipelineColorBlendAttachmentState {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    auto pipeline_color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &pipeline_color_blend_attachment_state,
    };

    std::vector<vk::DynamicState> dynamic_states = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    auto dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo {
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo {
        .setLayoutCount = 1,
        .pSetLayouts = &*m_descriptor_set_layout,
        .pushConstantRangeCount = 0,
    };

    m_pipeline_layout = vk::raii::PipelineLayout(m_device, pipeline_layout_create_info);

    auto pipeline_rendering_create_info = vk::PipelineRenderingCreateInfo {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &m_swap_chain_surface_format.format,
    };

    vk::VertexInputBindingDescription binding_description = Vertex::get_binding_description();
    std::vector<vk::VertexInputAttributeDescription> attribute_descriptions =
        Vertex::get_attribute_descriptions();

    auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
        .pVertexAttributeDescriptions = attribute_descriptions.data(),
    };

    auto graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo {
        .pNext = &pipeline_rendering_create_info,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pViewportState = &pipeline_viewport_state_info,
        .pRasterizationState = &pipeline_rasterization_state_create_info,
        .pMultisampleState = &pipeline_multi_sample_state_create_info,
        .pColorBlendState = &pipeline_color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = m_pipeline_layout,
        .renderPass = nullptr,
    };

    m_graphics_pipeline = vk::raii::Pipeline(m_device, nullptr, graphics_pipeline_create_info);
}

auto Renderer::create_command_pool() -> void {
    auto command_pool_create_info = vk::CommandPoolCreateInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_family_indices.graphics_family,
    };

    m_command_pool = vk::raii::CommandPool(m_device, command_pool_create_info);
}

auto Renderer::create_command_buffers() -> void {
    m_command_buffers.clear();

    auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo {
        .commandPool = m_command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    m_command_buffers = vk::raii::CommandBuffers(m_device, command_buffer_allocate_info);
}

auto Renderer::record_command_buffer(uint32_t image_index) -> void {
    vk::raii::CommandBuffer& command_buffer = m_command_buffers[frame_index];

    command_buffer.begin(vk::CommandBufferBeginInfo {});

    transition_image_layout(
        command_buffer,
        m_swap_chain_images[image_index],
        ImageTransitionInfo::undefined_to_color_attachment_optimal()
    );

    auto clear_color_value = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f});

    auto rendering_attachment_info = vk::RenderingAttachmentInfo {
        .imageView = m_swap_chain_image_views[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearValue {.color = clear_color_value},
    };

    auto clear_depth_value = vk::ClearDepthStencilValue(1.0f, 0.0f);

    auto depth_attachment_info = vk::RenderingAttachmentInfo {
        .imageView = m_depth_image_view,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearValue {.depthStencil = clear_depth_value},
    };

    auto rendering_info = vk::RenderingInfo {
        .renderArea =
            vk::Rect2D {
                .offset = vk::Offset2D {.x = 0, .y = 0},
                .extent = m_swap_chain_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &rendering_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    command_buffer.beginRendering(rendering_info);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline);

    command_buffer.setViewport(
        0,
        vk::Viewport {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_swap_chain_extent.width),
            .height = static_cast<float>(m_swap_chain_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        }
    );

    command_buffer.setScissor(
        0,
        vk::Rect2D {
            .offset = vk::Offset2D {.x = 0, .y = 0},
            .extent = m_swap_chain_extent,
        }
    );

    const vk::DescriptorSet& descriptor_set = m_descriptor_sets[frame_index];

    command_buffer.bindVertexBuffers(0, *m_vertex_buffer, {0});
    command_buffer.bindIndexBuffer(*m_index_buffer, 0, vk::IndexType::eUint16);
    command_buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, m_pipeline_layout, 0, descriptor_set, nullptr
    );

    command_buffer.drawIndexed(INDICES.size(), 1, 0, 0, 0);

    ImGuiUtils::render(command_buffer);

    command_buffer.endRendering();

    transition_image_layout(
        command_buffer,
        m_swap_chain_images[image_index],
        ImageTransitionInfo::color_attachment_optimal_to_present_src()
    );

    command_buffer.end();
}

auto Renderer::create_sync_objects() -> void {
    assert(m_present_complete_semaphores.empty());
    assert(m_render_finished_semaphores.empty());
    assert(in_flight_fences.empty());

    for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
        m_render_finished_semaphores.emplace_back(m_device, vk::SemaphoreCreateInfo {});
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_present_complete_semaphores.emplace_back(m_device, vk::SemaphoreCreateInfo {});

        auto fence_create_info = vk::FenceCreateInfo {
            .flags = vk::FenceCreateFlagBits::eSignaled,
        };

        in_flight_fences.emplace_back(m_device, fence_create_info);
    }
}

auto Renderer::create_vertex_buffer() -> void {
    size_t buffer_size = sizeof(VERTICES.front()) * VERTICES.size();
    m_vertex_buffer = create_device_vertex_buffer(VERTICES.data(), buffer_size);
}

/**
 * This just creates the buffer for the demo
 */
auto Renderer::create_index_buffer() -> void {
    const vk::DeviceSize size = INDICES.size() * sizeof(INDICES.front());
    m_index_buffer = create_device_index_buffer(INDICES.data(), size);
}

auto Renderer::draw_frame() -> void {
    auto fence_result = m_device.waitForFences(
        *in_flight_fences[frame_index], vk::True, std::numeric_limits<uint64_t>::max()
    );

    auto [result, image_index] = m_swap_chain.acquireNextImage(
        std::numeric_limits<uint64_t>::max(), *m_present_complete_semaphores[frame_index], nullptr
    );

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreate_swap_chain();
        return;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_device.resetFences(*in_flight_fences[frame_index]);

    update_uniform_buffer(frame_index);

    m_command_buffers[frame_index].reset();
    record_command_buffer(image_index);

    vk::PipelineStageFlags wait_destination_stage_mask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;

    // see https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html

    auto submit_info = vk::SubmitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_present_complete_semaphores[frame_index],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*m_command_buffers[frame_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_render_finished_semaphores[image_index],
    };

    m_graphics_queue.submit(submit_info, *in_flight_fences[frame_index]);

    auto present_info = vk::PresentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_render_finished_semaphores[image_index],
        .swapchainCount = 1,
        .pSwapchains = &*m_swap_chain,
        .pImageIndices = &image_index,
    };

    vk::Result present_result = m_presentation_queue.presentKHR(present_info);

    if (present_result == vk::Result::eErrorOutOfDateKHR ||
        present_result == vk::Result::eSuboptimalKHR) {
        recreate_swap_chain();
    }

    frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto Renderer::wait_idle() -> void {
    m_device.waitIdle();
}

auto Renderer::create_buffer(vk::DeviceSize size, BufferInfo create_info) const
    -> vma::raii::Buffer {
    const auto buffer_create_info = vk::BufferCreateInfo {
        .size = size,
        .usage = create_info.buffer_usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    const auto allocation_info = vma::AllocationCreateInfo {
        .flags = create_info.allocation_flags,
        .usage = create_info.allocation_usage,
    };

    vma::raii::Buffer buffer = m_allocator.createBuffer(buffer_create_info, allocation_info);

    // if (data.has_value()) {
    //     const vma::raii::Allocation& allocation = buffer.getAllocation();
    //     void* allocated_data = allocation.map();
    //     std::memcpy(allocated_data, data.value(), size);
    //     allocation.unmap();
    // }

    return buffer;
}

auto Renderer::create_and_map_buffer(const void* data, vk::DeviceSize size, BufferInfo info)
    -> vma::raii::Buffer {
    vma::raii::Buffer buffer = create_buffer(size, info);

    const vma::raii::Allocation& allocation = buffer.getAllocation();
    void* allocated_data = allocation.map();
    std::memcpy(allocated_data, data, size);
    allocation.unmap();

    return buffer;
}

// TODO: sync these "properly"
auto Renderer::create_device_vertex_buffer(const void* data, vk::DeviceSize size)
    -> vma::raii::Buffer {
    vma::raii::Buffer staging_buffer =
        create_and_map_buffer(data, size, BufferInfo::vertex_staging_buffer());

    vma::raii::Buffer device_buffer = create_buffer(size, BufferInfo::vertex_device_buffer());

    copy_buffer_waited(staging_buffer, device_buffer, size);

    return device_buffer;
}

auto Renderer::create_device_index_buffer(const void* data, vk::DeviceSize size)
    -> vma::raii::Buffer {
    vma::raii::Buffer staging_buffer =
        create_and_map_buffer(data, size, BufferInfo::index_staging_buffer());

    vma::raii::Buffer device_buffer = create_buffer(size, BufferInfo::index_device_buffer());

    copy_buffer_waited(staging_buffer, device_buffer, size);

    return device_buffer;
}

auto Renderer::copy_buffer_waited(vk::raii::Buffer& from, vk::raii::Buffer& to, vk::DeviceSize size)
    -> void {
    run_single_time_commands([&](vk::raii::CommandBuffer& command_buffer) {
        command_buffer.copyBuffer(
            from,
            to,
            vk::BufferCopy {
                .srcOffset = 0,
                .dstOffset = 0,
                .size = size,
            }
        );
    });
}

auto Renderer::create_descriptor_set_layout() -> void {
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings = {
        vk::DescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType =
                vk::DescriptorType::eUniformBuffer,  // which kind of shaders it binds to
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
        },

        vk::DescriptorSetLayoutBinding {
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
        }
    };

    auto descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo {
        .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
        .pBindings = layout_bindings.data(),
    };

    m_descriptor_set_layout =
        vk::raii::DescriptorSetLayout(m_device, descriptor_set_layout_create_info);
}

auto Renderer::create_uniform_buffers() -> void {
    vk::DeviceSize size = sizeof(UniformBuffer);

    m_uniform_buffers.clear();
    m_uniform_buffers_mapped.clear();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vma::raii::Buffer buffer = create_buffer(size, BufferInfo::uniform_buffer());
        const vma::raii::Allocation& allocation = buffer.getAllocation();
        void* mapped = allocation.map();

        m_uniform_buffers_mapped.emplace_back(mapped);
        m_uniform_buffers.emplace_back(std::move(buffer));
    }
}

auto Renderer::update_clocks_before_draw() -> void {
    m_frame_start_time = std::chrono::high_resolution_clock::now();

    m_elapsed_time = std::chrono::duration<float, std::chrono::seconds::period>(
                         m_frame_start_time - m_start_time
    )
                         .count();
}

auto Renderer::update_clocks_after_draw() -> void {
    std::chrono::high_resolution_clock::time_point frame_end_time =
        std::chrono::high_resolution_clock::now();

    m_last_frame_time = std::chrono::duration<float, std::chrono::milliseconds::period>(
                            frame_end_time - m_frame_start_time
    )
                            .count();
}

auto Renderer::update_uniform_buffer(uint32_t current_image) -> void {
    glm::mat4 model =
        rotate(glm::mat4(1.0f), m_elapsed_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 view = lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(m_swap_chain_extent.width) /
            static_cast<float>(m_swap_chain_extent.height),
        0.1f,
        10.0f
    );

    // flip y
    projection[1][1] *= -1;

    UniformBuffer uniform_buffer = {
        .model = model,
        .view = view,
        .projection = projection,
    };

    void* current_buffer = m_uniform_buffers_mapped[current_image];
    std::memcpy(current_buffer, &uniform_buffer, sizeof(UniformBuffer));
}

auto Renderer::create_descriptor_pool() -> void {
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampledImage,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampler,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE,
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT,
        }

    };

    // auto desciptor_pool_size = vk::DescriptorPoolSize {
    //     .type = vk::DescriptorType::eUniformBuffer,
    //     .descriptorCount = MAX_FRAMES_IN_FLIGHT,
    // };

    auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT * 2 + IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE +
                   IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    m_descriptor_pool = vk::raii::DescriptorPool(m_device, descriptor_pool_create_info);
}

auto Renderer::create_descriptor_sets() -> void {
    auto layouts =
        std::vector<vk::DescriptorSetLayout>(MAX_FRAMES_IN_FLIGHT, *m_descriptor_set_layout);

    auto descriptor_set_allocate_info = vk::DescriptorSetAllocateInfo {
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    m_descriptor_sets.clear();
    m_descriptor_sets = m_device.allocateDescriptorSets(descriptor_set_allocate_info);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto descriptor_buffer_info = vk::DescriptorBufferInfo {
            .buffer = m_uniform_buffers[i],
            .offset = 0,
            .range = sizeof(UniformBuffer),
        };

        auto desciptor_image_info = vk::DescriptorImageInfo {
            .sampler = m_texture_sampler,
            .imageView = m_texture_image_view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        std::vector<vk::WriteDescriptorSet> write_descriptor_sets = {
            vk::WriteDescriptorSet {
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &descriptor_buffer_info,
            },
            vk::WriteDescriptorSet {
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &desciptor_image_info,
            }
        };

        m_device.updateDescriptorSets(write_descriptor_sets, {});
    }
}

/**
 * @brief Load the .ktx texture at file path to the m_texture_image and point an image view
 * m_texture_image_view to it
 *
 * @param file_path
 */
auto Renderer::load_texture(vk::raii::CommandBuffer& command_buffer, const std::string& file_path)
    -> void {
    KtxImage texture = KtxImage::from_file_name(file_path);

    std::println(
        "loading image: {} with width: {}px, height {}px", file_path, texture.width, texture.height
    );

    m_texture_image = create_image(texture.width, texture.height, ImageInfo::r8g8b8a8_srgb());

    vma::raii::Buffer staging_buffer =
        create_and_map_buffer(texture.data, texture.size, BufferInfo::image_staging_buffer());

    transition_image_layout(
        command_buffer, m_texture_image, ImageTransitionInfo::undefined_to_dst_optimal()
    );

    copy_buffer_to_image(
        command_buffer, staging_buffer, m_texture_image, texture.width, texture.height
    );

    transition_image_layout(
        command_buffer, m_texture_image, ImageTransitionInfo::dst_optimal_to_shader_optimal()
    );

    m_texture_image_view = create_image_view(m_texture_image, ImageViewInfo::eR8G8B8A8Srgb_color());
}

/// Create and allocate (on the GPU) an image
auto Renderer::create_image(uint32_t width, uint32_t height, ImageInfo info) -> vma::raii::Image {
    auto image_create_info = vk::ImageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = info.format,
        .extent =
            vk::Extent3D {
                .width = width,
                .height = height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = info.tiling,
        .usage = info.usage_flags,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    auto allocation_create_info = vma::AllocationCreateInfo {
        .usage = vma::MemoryUsage::eAutoPreferDevice,
    };

    vma::raii::Image image =
        m_allocator.createImage(image_create_info, allocation_create_info, nullptr);

    return image;
}

auto Renderer::transition_image_layout(
    vk::raii::CommandBuffer& command_buffer, const vk::Image& image, const ImageTransitionInfo& info
) -> void {
    auto barrier = vk::ImageMemoryBarrier2 {
        .srcStageMask = info.source_stage_flags,
        .srcAccessMask = info.source_access_flags,
        .dstStageMask = info.destination_stage_flags,
        .dstAccessMask = info.destination_accesss_flags,
        .oldLayout = info.source,
        .newLayout = info.destination,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = info.subresource_range,
    };

    auto dependency_info = vk::DependencyInfo {
        .memoryBarrierCount = 0,
        .bufferMemoryBarrierCount = 0,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    command_buffer.pipelineBarrier2(dependency_info);
}

auto Renderer::create_and_map_image(void* data, uint32_t width, uint32_t height, ImageInfo info)
    -> vma::raii::Image {
    vma::raii::Image empty_image = create_image(width, height, info);
    size_t size = width * height * 4;

    vma::raii::Buffer staging_buffer =
        create_and_map_buffer(data, size, BufferInfo::image_staging_buffer());
}

auto Renderer::copy_buffer_to_image(
    const vk::raii::CommandBuffer& command_buffer,
    const vma::raii::Buffer& buffer,
    vma::raii::Image& destination,
    uint32_t width,
    uint32_t height
) -> void {
    auto buffer_image_copy = vk::BufferImageCopy {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            vk::ImageSubresourceLayers {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset =
            vk::Offset3D {
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .imageExtent = vk::Extent3D {
            .width = width,
            .height = height,
            .depth = 1,
        },
    };

    command_buffer.copyBufferToImage(
        buffer, destination, vk::ImageLayout::eTransferDstOptimal, {buffer_image_copy}
    );
};

/**
 * @brief Create a new command buffer and run the given commands inside of it, run waitIdle() after
 *
 * @param commands
 */
auto Renderer::run_single_time_commands(
    std::function<void(vk::raii::CommandBuffer& command_buffer)> commands
) -> void {
    auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo {
        .commandPool = m_command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    vk::raii::CommandBuffer command_buffer =
        std::move(m_device.allocateCommandBuffers(command_buffer_allocate_info).front());

    auto command_buffer_begin_info = vk::CommandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    command_buffer.begin(command_buffer_begin_info);

    commands(command_buffer);

    command_buffer.end();

    auto submit_info = vk::SubmitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer,
    };

    m_graphics_queue.submit(submit_info, nullptr);
    m_graphics_queue.waitIdle();
}

auto Renderer::create_image_sampler() -> void {
    auto sampler_info = vk::SamplerCreateInfo {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .maxAnisotropy = m_max_sampler_anisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .unnormalizedCoordinates = vk::False,
    };

    m_texture_sampler = vk::raii::Sampler(m_device, sampler_info);
}

auto Renderer::find_depth_format() -> vk::Format {
    return find_supported_image_format(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

auto Renderer::create_depth_resources() -> void {
    vk::Format depth_format = find_depth_format();

    m_depth_image = create_image(
        m_swap_chain_extent.width, m_swap_chain_extent.height, ImageInfo::depth_format(depth_format)
    );

    m_depth_image_view = create_image_view(m_depth_image, ImageViewInfo::depth(depth_format));
    //  m_depth_image_view = create_image_view(m_depth_image, vk::Format format)
}

auto Renderer::find_supported_image_format(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features
) -> vk::Format {
    for (const auto& format : candidates) {
        vk::FormatProperties properties = m_physical_device.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (properties.linearTilingFeatures & features) == features) {
            return format;
        }

        if (tiling == vk::ImageTiling::eOptimal &&
            (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

auto Renderer::prepare() -> void {
    m_start_time = std::chrono::high_resolution_clock::now();
    init_vulkan();
}

auto Renderer::main_loop() -> void {
}

auto Renderer::cleanup() -> void {
    m_device.waitIdle();

    for (size_t i = 0; i < m_uniform_buffers.size(); i++) {
        m_uniform_buffers[i].getAllocation().unmap();
        m_uniform_buffers_mapped[i] = nullptr;
    }

    ImGuiUtils::cleanup();

    cleanup_swap_chain();
}

auto Renderer::cleanup_swap_chain() -> void {
    m_swap_chain_images.clear();
    m_swap_chain = nullptr;
}

auto Renderer::recreate_swap_chain() -> void {
    m_device.waitIdle();

    cleanup_swap_chain();

    create_swap_chain();
    create_swapchain_image_views();
}

auto Renderer::create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo {
    // auto rendering_info = vk::PipelineRenderingCreateInfoKHR {
    //     .colorAttachmentCount = 1,
    //     .pColorAttachmentFormats = &m_swap_chain_surface_format.format,
    // };

    auto init_info = ImGui_ImplVulkan_InitInfo {
        .Instance = *m_instance,
        .PhysicalDevice = *m_physical_device,
        .Device = *m_device,
        .QueueFamily = m_family_indices.graphics_family,
        .Queue = *m_graphics_queue,
        .DescriptorPool = *m_descriptor_pool,
        .MinImageCount = 3,
        .ImageCount = static_cast<uint32_t>(m_swap_chain_images.size()),
        .PipelineInfoMain =
            ImGui_ImplVulkan_PipelineInfo {
                .PipelineRenderingCreateInfo =
                    vk::PipelineRenderingCreateInfoKHR {
                        .colorAttachmentCount = 1,
                        .pColorAttachmentFormats = &m_swap_chain_surface_format.format,
                    },
            },
        .UseDynamicRendering = true,

    };

    return init_info;
}

Renderer::~Renderer() {
    cleanup();
}
