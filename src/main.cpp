#include <print>

#include "Renderer.hpp"
#include "Window.hpp"

auto main() -> int {
    auto window = Window("Vulkan Window", 800, 600);

    auto renderer_settings = RendererSettings {
        .enable_validation = true,
        .validation_log_level = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
        .prefer_discrete_gpu = true,
    };
    auto renderer = Renderer(window, renderer_settings);

    renderer.run();
}