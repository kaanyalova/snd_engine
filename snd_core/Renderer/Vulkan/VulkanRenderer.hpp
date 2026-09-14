#pragma once

#include <imgui_impl_vulkan.h>

#include <optional>

#include "../../Resources/Scene/SceneData.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Renderer/Vulkan/VulkanSwapchain.hpp"

struct RendererSettings {
    bool enable_validation = true;
    GpuPreference gpu_preference = GpuPreference::Integrated;
};

class VulkanRenderer {
  public:
    VulkanRenderer(Window& window, const RendererSettings& settings);

    auto recreate_swapchain() -> void;
    auto create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo;
    auto process() -> void;

    auto get_device() -> VulkanDevice& { return m_device.value(); }
    auto get_swapchain() -> VulkanSwapchain& { return m_swapchain.value(); }
    auto get_imgui_descriptor_pool() -> const vk::DescriptorPool& {
        return m_imgui_descriptor_pool;
    }
    auto get_renderer_descriptor_pool() -> const vk::DescriptorPool& {
        return m_renderer_descriptor_pool;
    }

    auto get_gpu_maximum_descriptor_set_count() -> uint32_t;
    static auto convert_to_vulkan_image_format(ImageFormat imageFormat) -> vk::Format;
    auto allocate_image_descriptors(const std::vector<vk::raii::Image>& images) -> void;

  private:
    static constexpr uint32_t DESCRIPTOR_POOL_MAX_SAMPLED_IMAGE_COUNT = 8000;
    static constexpr uint32_t DESCRIPTOR_POOL_MAX_SAMPLER_COUNT = 512;

    // the textures sets starts with the capacity of 1024, and it grows until
    // it hits the gpu limit, which it won't before you run out of memory
    uint32_t m_gpu_max_descriptor_set_count = 0;
    static constexpr uint32_t DESCRIPTOR_SET_INITIAL_COUNT = 1024;
    static constexpr float DESCRIPTOR_SET_IMAGES_REALLOCATE_SCALING_FACTOR = 1.5;

    std::optional<VulkanDevice> m_device = std::nullopt;
    std::optional<VulkanSwapchain> m_swapchain = std::nullopt;
    vk::raii::DescriptorPool m_imgui_descriptor_pool = nullptr;
    vk::raii::DescriptorPool m_renderer_descriptor_pool = nullptr;

    vk::raii::DescriptorSetLayout m_renderer_descriptor_set_layout_samplers = nullptr;
    vk::raii::DescriptorSetLayout m_renderer_descriptor_set_layout_images = nullptr;

    RendererSettings m_settings;

    Window& m_window;

    auto create_device() -> void;
    auto create_swapchain() -> void;

    auto create_imgui_descriptor_pool() -> void;

    auto create_renderer_descriptor_pool() -> void;
    auto create_renderer_descriptor_set_layouts() -> void;
    auto allocate_descriptor_set() -> void;
};
