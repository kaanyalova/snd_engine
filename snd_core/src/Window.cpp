#include "Window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include <cstdint>
#include <print>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

#include "Renderer/Vulkan/VulkanDevice.hpp"

Window::Window(const char* title, uint32_t width, uint32_t height) {
    bool result = SDL_Init(SDL_INIT_VIDEO);

    if (result != true) {
        const char* sdl_error = SDL_GetError();
        throw std::runtime_error(std::format("Failed to initialize SDL: {}", sdl_error));
    }

    constexpr SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    m_sdl_window = SDL_CreateWindow(title, width, height, flags);

    if (m_sdl_window == nullptr) {
        const char* error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error(std::format("Failed to create SDL window, Error: {}", error));
    }
}

auto Window::get_required_vulkan_extensions() -> std::vector<const char*> {
    uint32_t extension_count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    auto extensions_vector = std::vector<const char*>(extensions, extensions + extension_count);
    return extensions_vector;
}

auto Window::poll_event() -> SDL_Event {
    SDL_Event event;
    SDL_PollEvent(&event);
    return event;
}

auto Window::create_surface(VulkanDevice& device) -> vk::raii::SurfaceKHR {
    VkSurfaceKHR surface;

    bool result = SDL_Vulkan_CreateSurface(m_sdl_window, *device.get_instance(), nullptr, &surface);

    if (result != true) {
        throw std::runtime_error("Failed to create Vulkan surface");
    }

    return vk::raii::SurfaceKHR(device.get_instance(), surface);
}

auto Window::sdl_window() -> SDL_Window& {
    return *m_sdl_window;
}

auto Window::size() -> std::pair<uint32_t, uint32_t> {
    int width;
    int height;

    SDL_GetWindowSizeInPixels(m_sdl_window, &width, &height);
    return std::pair(width, height);
}

Window::~Window() {
    std::println("Destroying the window");
    SDL_DestroyWindow(m_sdl_window);
    SDL_Quit();
}
