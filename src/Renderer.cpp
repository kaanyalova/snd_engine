#include "Renderer.hpp"

#include <SDL3/SDL_video.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <print>
#include <ranges>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "FileUtils.hpp"

Renderer::Renderer(Window& window, RendererSettings settings)
    : m_required_extensions(std::move(settings.required_extensions))
    , m_validation_layers(std::move(settings.validation_layers))
    , m_enable_validation(settings.enable_validation)
    , m_validation_log_level(settings.validation_log_level)
    , m_validation_message_types(settings.validation_message_types)
    , m_prefer_discrete_gpu(settings.prefer_discrete_gpu)
    , m_window(window) {
}

auto Renderer::init_vulkan() -> void {
    create_instance();
    if (m_enable_validation) {
        setup_debug_messenger();
    }
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swap_chain();
    create_image_views();
    create_graphics_pipeline();
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

    m_instance = vk::raii::Instance(m_context, create_info);
}

auto Renderer::are_required_extensions_supported_by_instance() -> bool {
    std::vector<vk::ExtensionProperties> extension_properties =
        m_context.enumerateInstanceExtensionProperties();

    return std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
        return std::ranges::any_of(
            extension_properties, [&](const vk::ExtensionProperties& available_extension) -> bool {
                return std::strcmp(required_extension, available_extension.extensionName) == 0;
            }
        );
    });
}

auto Renderer::are_required_validation_layers_supported_by_instance() -> bool {
    std::vector<vk::LayerProperties> layer_properties =
        m_context.enumerateInstanceLayerProperties();

    return std::ranges::all_of(m_validation_layers, [&](const char* required_layer) -> bool {
        return std::ranges::any_of(
            layer_properties, [&](const vk::LayerProperties& available_layer) -> bool {
                return std::strcmp(required_layer, available_layer.layerName) == 0;
            }
        );
    });
}

auto Renderer::get_validation_layers() -> std::vector<const char*> {
    if (m_enable_validation) {
        if (!are_required_extensions_supported_by_instance()) {
            throw std::runtime_error("some validation layers are not supported");
        }
        return std::move(m_validation_layers);
    }
    return {};
}

auto Renderer::is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool {
    const vk::PhysicalDeviceProperties device_properties = device.getProperties();
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();
    std::vector<vk::ExtensionProperties> device_extension_properties =
        device.enumerateDeviceExtensionProperties();

    const bool is_api_version_suitable = device_properties.apiVersion >= vk::ApiVersion13;
    const bool has_graphics_queue = std::ranges::any_of(
        queue_family_properties, [](const vk::QueueFamilyProperties& properties) {
            return static_cast<bool>(properties.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    // const bool supports_required_extensions =
    //     std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
    //         return std::ranges::any_of(
    //             device_extension_properties,
    //             [&](const vk::ExtensionProperties& available_extension) {
    //                 return std::strcmp(available_extension.extensionName, required_extension) ==
    //                 0;
    //             }
    //         );
    //     });

    const bool is_suitable =
        is_api_version_suitable && has_graphics_queue;  // && supports_required_extensions;

    return is_suitable;
}

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
    bool also_supports_presentation = device.getSurfaceSupportKHR(graphics_family_index, *m_surface);

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

    m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(messenger_create_info);
}

auto Renderer::pick_physical_device() -> void {
    std::vector<vk::raii::PhysicalDevice> physical_devices = m_instance.enumeratePhysicalDevices();

    if (m_prefer_discrete_gpu) {
        const auto discrete_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                       is_device_suitable(device);
            });

        if (discrete_gpu_it != physical_devices.end()) {
            m_physical_device = *discrete_gpu_it;
            return;
        }
    } else {
        // pick igpu over any gpu if m_prefer_discrete_gpu is false
        const auto integrated_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType ==
                           vk::PhysicalDeviceType::eIntegratedGpu &&
                       is_device_suitable(device);
            });

        if (integrated_gpu_it != physical_devices.end()) {
            m_physical_device = *integrated_gpu_it;
            return;
        }
    }

    // cannot find the preferred gpu, fallback to any one that works
    auto fallback_it =
        std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
            return is_device_suitable(device);
        });

    if (fallback_it != physical_devices.end()) {
        m_physical_device = *fallback_it;
        return;
    }

    // no gpus are suitable
    throw std::runtime_error("failed to find a suitable GPU");
}

auto Renderer::create_logical_device() -> void {
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        m_physical_device.getQueueFamilyProperties();

    m_family_indices = find_queue_families(m_physical_device);
    float queue_priority = 1.0f;

    auto device_queue_create_info = vk::DeviceQueueCreateInfo {
        .queueFamilyIndex = m_family_indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    using DeviceFeaturesChain = vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>;

    auto feature_chain = DeviceFeaturesChain {
        vk::PhysicalDeviceFeatures2 {},
        vk::PhysicalDeviceVulkan13Features {.dynamicRendering = true},
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {.extendedDynamicState = true},
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

    m_device = vk::raii::Device(m_physical_device, device_create_info);
    m_graphics_queue = vk::raii::Queue(m_device, m_family_indices.graphics_family, 0);
    m_presentation_queue = vk::raii::Queue(m_device, m_family_indices.presentation_family, 0);
}

auto Renderer::create_surface() -> void {
    vk::SurfaceKHR surface = m_window.create_surface(m_instance);

    if (surface == nullptr) {
        throw std::runtime_error("vulkan surface returned nullptr");
    }

    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}

auto Renderer::debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
    void* _user_data
) -> vk::Bool32 {
    std::println("vulkan validation layer: {}", callback_data->pMessage);
    return vk::False;
}

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
    auto surface_capabilities = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);
    m_swap_chain_extent = choose_swap_extent(surface_capabilities);

    m_swap_chain_surface_format =
        choose_swap_surface_format(m_physical_device.getSurfaceFormatsKHR(m_surface));

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
        choose_swap_present_mode(m_physical_device.getSurfacePresentModesKHR(*m_surface));

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

auto Renderer::create_image_views() -> void {
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

[[nodiscard]] auto Renderer::create_shader_module(const std::vector<char>& code) const
    -> vk::raii::ShaderModule {
    auto shader_module_create_info = vk::ShaderModuleCreateInfo {
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    auto shader_module = vk::raii::ShaderModule(m_device, shader_module_create_info);
    return shader_module;
}

auto Renderer::create_graphics_pipeline() -> void {
    std::vector<char> shader_code = FileUtils::read_file("shaders/triangle.spv");
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
        .frontFace = vk::FrontFace::eClockwise,
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

    auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    m_pipeline_layout = vk::raii::PipelineLayout(m_device, pipeline_layout_create_info);

    auto pipeline_rendering_create_info = vk::PipelineRenderingCreateInfo {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &m_swap_chain_surface_format.format,
    };

    auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {};

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
        .pDynamicState = nullptr,
        .layout = m_pipeline_layout,
        .renderPass = nullptr,
    };

    m_graphics_pipeline = vk::raii::Pipeline(m_device, nullptr, graphics_pipeline_create_info);
}

auto Renderer::run() -> void {
    init_vulkan();
}

auto Renderer::main_loop() -> void {
}
