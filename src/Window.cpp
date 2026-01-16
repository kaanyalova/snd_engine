module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
module Window;

Window::Window(const char* title, uint32_t width, uint32_t height) {
    bool result = SDL_Init(SDL_INIT_VIDEO);

    if (result != 0) {
        throw std::runtime_error("Failed to initialize SDL");
    }

    constexpr SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    m_sdl_window = SDL_CreateWindow(title, width, height, flags);

    if (m_sdl_window == nullptr) {
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL window");
    }
}

auto Window::get_required_vulkan_extensions() -> std::vector<const char*> {
    uint32_t extension_count = 0;
    const char* const* extenions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    auto extensions_vector = std::vector<const char*>(extenions, extenions + extension_count);
    return extensions_vector;
}

auto Window::poll_event() -> SDL_Event {
    SDL_Event event;
    SDL_PollEvent(&event);
    return event;
}

Window::~Window() {
    SDL_DestroyWindow(m_sdl_window);
    SDL_Quit();
}
