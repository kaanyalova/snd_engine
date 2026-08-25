#include <SDL3/SDL_events.h>
#include <imgui_impl_vulkan.h>

#include <print>

#include "RendererInfoGui.hpp"
#include "Utils/ImGuiUtils.hpp"
#include "Window.hpp"

auto main() -> int {
    /*
    auto window = Window("Vulkan Window", 800, 600);

    auto renderer_settings = RendererSettings {
        .enable_validation = true,
        //.validation_log_level = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
        .gpu_preference = GpuPreference::Integrated,
    };

    try {
        auto renderer = VulkanRenderer(window, renderer_settings);

        ImGui_ImplVulkan_InitInfo imgui_vulkan_init_info = renderer.create_imgui_init_info();
        ImGuiUtils::initialize(window, imgui_vulkan_init_info);

        while (true) {
            SDL_Event event = window.poll_event();

            if (event.type == SDL_EVENT_QUIT) {
                break;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                renderer.recreate_swapchain();
            }

            ImGuiUtils::process_event(event);
            ImGuiUtils::update();
            RendererInfoGui::process();

            renderer.process();
        }
    }

    catch (const std::runtime_error& e) {
        std::println("Failed to create renderer: {}", e.what());
        return EXIT_FAILURE;
    }
*/
}
