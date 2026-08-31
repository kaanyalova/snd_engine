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
    auto load_scene_data(const SceneData& scene_data) -> void;
    auto create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo;
    auto process() -> void;

  private:
    auto create_device() -> void;
    auto create_swapchain() -> void;
    auto render_scene(SceneData& scene_data) -> void;

    std::optional<VulkanDevice> m_device = std::nullopt;
    std::optional<VulkanSwapchain> m_swapchain = std::nullopt;

    RendererSettings m_settings;

    Window& m_window;
};
