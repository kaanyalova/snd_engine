#pragma once
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

class VulkanRenderer;

class VulkanPrimitiveBuffer : public VulkanBuffer {
  public:
    explicit VulkanPrimitiveBuffer(VulkanRenderer& renderer, std::span<const Primitive> primitives);

  private:
    auto load_to_gpu() -> void;

    std::span<const Primitive> m_primitives = {};
};
