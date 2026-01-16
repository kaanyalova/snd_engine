module;
#include <SDL3/SDL.h>

export module Window;

import std;

class Window {
  public:
    Window(const char* title, uint32_t width, uint32_t height);
    auto get_required_vulkan_extensions() -> std::vector<const char*>;

    auto poll_event() -> SDL_Event;

    ~Window();

  private:
    SDL_Window* m_sdl_window = nullptr;
    bool m_is_running = true;
};