#include <print>

#include "Renderer.hpp"
#include "Window.hpp"

auto main() -> int {
    auto window = Window("Vulkan Window", 800, 600);

    auto renderer_settings = RendererSettings{
        .enable_validation = true,
        //.validation_log_level = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
        .prefer_discrete_gpu = true,
    };

    try {
        auto renderer = Renderer(window, renderer_settings);
        renderer.prepare();

        while (true) {
            SDL_Event event = window.poll_event();

            if (event.type == SDL_EVENT_QUIT) {
                break;
            }

            renderer.draw_frame();
        }

        renderer.wait_idle();



    } catch (const std::runtime_error &e) {
        std::println("Failed to create renderer: {}", e.what());
        return EXIT_FAILURE;
    }
}
