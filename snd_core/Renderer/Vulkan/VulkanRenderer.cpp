#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"

#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Renderer/Vulkan/VulkanSwapchain.hpp"

VulkanRenderer::VulkanRenderer(Window& window, const RendererSettings& settings)
    : m_window(window) {
    create_device();
    create_swapchain();
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

auto VulkanRenderer::recreate_swapchain() -> void {
    m_swapchain.reset();
    create_swapchain();
}

auto VulkanRenderer::create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo {
    auto init_info = ImGui_ImplVulkan_InitInfo {
        .Instance = *m_device->get_instance(),
        .PhysicalDevice = *m_device->get_physical_device(),
        .Device = *m_device->get_device(),
        .QueueFamily = m_device->get_family_indices().graphics,
        .Queue = *m_device->get_queues().graphics,
        .DescriptorPool = nullptr,  // todo
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