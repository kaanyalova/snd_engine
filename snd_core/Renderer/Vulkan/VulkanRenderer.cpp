#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"

#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Renderer/Vulkan/VulkanSwapchain.hpp"

VulkanRenderer::VulkanRenderer(Window& window, const RendererSettings& settings)
    : m_window(window) {
    create_device();
    create_swapchain();
}

auto VulkanRenderer::get_gpu_maximum_descriptor_set_count() -> uint32_t {
    // todo
    return 8000;
}
auto VulkanRenderer::convert_to_vulkan_image_format(ImageFormat imageFormat) -> vk::Format {
    switch (imageFormat) {
        case ImageFormat::Undefined:
            break;
        case ImageFormat::Rgba8:
            break;
        case ImageFormat::Rgb8:
            break;
    }
}
auto VulkanRenderer::create_device() -> void {
    auto device_create_info = DeviceCreationInfo {
        .enable_validation = true,
        .gpu_preference = GpuPreference::Integrated,

    };

    m_device.emplace(VulkanDevice(device_create_info, m_window));
}

auto VulkanRenderer::create_swapchain() -> void {
    m_swapchain.emplace(VulkanSwapchain(*m_device, m_window));
}
auto VulkanRenderer::create_imgui_descriptor_pool() -> void {
    std::array<vk::DescriptorPoolSize, 2> pool_sizes = {
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampledImage,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampler,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE,
        }
    };

    auto pool_create_info = vk::DescriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE +
                   IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE,
        .poolSizeCount = pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

    m_imgui_descriptor_pool = m_device->inner().createDescriptorPool(pool_create_info);
}
auto VulkanRenderer::create_renderer_descriptor_pool() -> void {
    auto pool_sizes = std::array<vk::DescriptorPoolSize, 2> {
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampledImage,
            .descriptorCount = DESCRIPTOR_POOL_MAX_SAMPLED_IMAGE_COUNT,
        },
        vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eSampler,
            .descriptorCount = DESCRIPTOR_POOL_MAX_SAMPLER_COUNT,
        }
    };

    auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo {
        .maxSets = 1,
        .poolSizeCount = pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

    m_renderer_descriptor_pool =
        m_device->inner().createDescriptorPool(descriptor_pool_create_info);
}
auto VulkanRenderer::create_renderer_descriptor_set_layouts() -> void {
    // create the image layout
    auto layout_binding_images = vk::DescriptorSetLayoutBinding {
        .descriptorType = vk::DescriptorType::eSampledImage,
        .descriptorCount = DESCRIPTOR_POOL_MAX_SAMPLED_IMAGE_COUNT,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };

    vk::DescriptorBindingFlags descriptor_layout_flag_images =
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount;

    vk::StructureChain<
        vk::DescriptorSetLayoutCreateInfo,
        vk::DescriptorSetLayoutBindingFlagsCreateInfo>
        descriptor_set_layout_create_info_images = {
            vk::DescriptorSetLayoutCreateInfo {
                .bindingCount = 1,
                .pBindings = &layout_binding_images,
            },
            vk::DescriptorSetLayoutBindingFlagsCreateInfo {
                .bindingCount = 1,
                .pBindingFlags = &descriptor_layout_flag_images,
            }
        };

    m_renderer_descriptor_set_layout_images = m_device->inner().createDescriptorSetLayout(
        descriptor_set_layout_create_info_images.get<vk::DescriptorSetLayoutCreateInfo>()
    );

    // create the sampler layout, at index 1
    auto layout_binding_samplers = vk::DescriptorSetLayoutBinding {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eSampler,
        .descriptorCount = DESCRIPTOR_POOL_MAX_SAMPLER_COUNT,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };

    vk::DescriptorBindingFlags descriptor_layout_flag_samplers =
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount;

    vk::StructureChain<
        vk::DescriptorSetLayoutCreateInfo,
        vk::DescriptorSetLayoutBindingFlagsCreateInfo>
        descriptor_set_layout_create_info_samplers = {
            vk::DescriptorSetLayoutCreateInfo {
                .bindingCount = 1,
                .pBindings = &layout_binding_images,
            },
            vk::DescriptorSetLayoutBindingFlagsCreateInfo {
                .bindingCount = 1,
                .pBindingFlags = &descriptor_layout_flag_images,
            }
        };

    m_renderer_descriptor_set_layout_samplers = m_device->inner().createDescriptorSetLayout(
        descriptor_set_layout_create_info_images.get<vk::DescriptorSetLayoutCreateInfo>()
    );
}

auto VulkanRenderer::allocate_image_descriptors(const std::vector<vk::raii::Image>& images)
    -> void {
    uint32_t descriptor_count = images.size();

    vk::StructureChain<
        vk::DescriptorSetAllocateInfo,
        vk::DescriptorSetVariableDescriptorCountAllocateInfo>
        descriptor_set_allocate_info = {
            vk::DescriptorSetAllocateInfo {
                .descriptorPool = m_renderer_descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &*m_renderer_descriptor_set_layout_images,
            },
            vk::DescriptorSetVariableDescriptorCountAllocateInfo {
                .descriptorSetCount = 1,
                .pDescriptorCounts = &descriptor_count,
            }
        };

    auto allocate = m_device->inner().allocateDescriptorSets(
        descriptor_set_allocate_info.get<vk::DescriptorSetAllocateInfo>()
    );
}

auto VulkanRenderer::recreate_swapchain() -> void {
    m_swapchain.reset();
    create_swapchain();
}

auto VulkanRenderer::create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo {
    auto init_info = ImGui_ImplVulkan_InitInfo {
        .Instance = *m_device->get_instance(),
        .PhysicalDevice = *m_device->get_physical_device(),
        .Device = *m_device->inner(),
        .QueueFamily = m_device->get_family_indices().graphics,
        .Queue = *m_device->get_queues().graphics,
        .DescriptorPool = *m_imgui_descriptor_pool,
        .MinImageCount = 3,
        .ImageCount = static_cast<uint32_t>(m_swapchain->get_images().size()),
        .PipelineInfoMain =
            ImGui_ImplVulkan_PipelineInfo {
                .PipelineRenderingCreateInfo =
                    vk::PipelineRenderingCreateInfoKHR {
                        .colorAttachmentCount = 1,
                        .pColorAttachmentFormats = &m_swapchain->get_surface_format().format,
                    },

            },
        .UseDynamicRendering = true,
    };

    return init_info;
}

auto VulkanRenderer::process() -> void {
}