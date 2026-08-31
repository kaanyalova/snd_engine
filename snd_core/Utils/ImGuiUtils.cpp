#include "snd_core/Utils/ImGuiUtils.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <vulkan/vulkan.hpp>

#include "snd_core/Window.hpp"

auto ImGuiUtils::initialize(Window& window, ImGui_ImplVulkan_InitInfo& vulkan_init_info) -> void {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags =
        io.ConfigFlags | ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplSDL3_InitForVulkan(&window.sdl_window());
    ImGui_ImplVulkan_Init(&vulkan_init_info);
}

auto ImGuiUtils::process_event(SDL_Event& event) -> void {
    ImGui_ImplSDL3_ProcessEvent(&event);
}

/**
 * @brief Updates the ImGui State
 * Should be called after the event loop, before the rendering
 */
auto ImGuiUtils::update() -> void {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

auto ImGuiUtils::render(vk::raii::CommandBuffer& command_buffer) -> void {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *command_buffer);
}

auto ImGuiUtils::cleanup() -> void {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}