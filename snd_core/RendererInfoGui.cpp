#include "RendererInfoGui.hpp"

#include <format>

#include "imgui.h"

auto RendererInfoGui::process() -> void {
    ImGui::BeginMainMenuBar();
    ImGui::Text("%fms", m_frame_time);

    ImGui::Text("%ffps", 1000.0f / m_frame_time);
    ImGui::EndMainMenuBar();
}

auto RendererInfoGui::set_frametime(float frame_time) -> void {
    m_frame_time = frame_time;
}
