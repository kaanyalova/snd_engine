#pragma once

#include <SDL3/SDL.h>

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class Window {
  public:
    Window(const char* title, uint32_t width, uint32_t height);
    auto get_required_vulkan_extensions() -> std::vector<const char*>;
    auto create_surface(vk::raii::Instance& instance) -> vk::raii::SurfaceKHR;

    auto poll_event() -> SDL_Event;

    auto sdl_window() -> SDL_Window&;

    ~Window();

  private:
    SDL_Window* m_sdl_window = nullptr;
    bool m_is_running = true;
};
