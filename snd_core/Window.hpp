#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

class Window {
  public:
    Window() = delete;

    Window(const char* title, uint32_t width, uint32_t height);
    auto get_required_vulkan_extensions() -> std::vector<const char*>;
    auto create_surface(VulkanDevice& device) -> vk::raii::SurfaceKHR;
    auto size() -> std::pair<uint32_t, uint32_t>;

    auto poll_event() -> SDL_Event;

    auto sdl_window() -> SDL_Window&;

    ~Window();

  private:
    SDL_Window* m_sdl_window = nullptr;
    bool m_is_running = true;
};
