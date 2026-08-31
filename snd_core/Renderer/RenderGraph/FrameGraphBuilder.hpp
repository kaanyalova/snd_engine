#pragma once
#include <functional>
#include <string_view>

#include "snd_core/Renderer/RenderGraph/FrameGraphResource.hpp"
#include "snd_core/Renderer/Renderer.hpp"

class FrameGraphBuilder {
    template <typename T>
    auto add_pass(
        std::string_view name,
        std::function<void(T& pass_data)> setup_callback,
        std::function<void(const T& pass_data, Renderer& renderer)> execute_callback
    ) -> FrameGraphResource {}
};