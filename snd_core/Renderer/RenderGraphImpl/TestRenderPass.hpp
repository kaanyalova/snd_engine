#pragma once
#include "snd_core/Renderer/RenderGraph/RenderPass.hpp"

class TestRenderPass : public RenderPass {
  public:
    auto setup() -> void override;
    auto execute() -> void override;
};
