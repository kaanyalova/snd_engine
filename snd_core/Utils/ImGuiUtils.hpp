#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <imgui_impl_vulkan.h>

#include <vulkan/vulkan.hpp>

#include "snd_core/Window.hpp"

class ImGuiUtils {
  public:
    static auto initialize(Window& window, ImGui_ImplVulkan_InitInfo& vulkan_init_info) -> void;
    static auto process_event(SDL_Event& event) -> void;
    static auto update() -> void;
    static auto render(vk::raii::CommandBuffer& command_buffer) -> void;
    static auto cleanup() -> void;
};
