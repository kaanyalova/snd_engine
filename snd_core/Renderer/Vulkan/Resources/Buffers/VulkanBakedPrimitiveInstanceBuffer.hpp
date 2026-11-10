#pragma once
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Resources/Scene/BakedScene.hpp"

class VulkanRenderer;

class VulkanBakedPrimitiveInstanceBuffer : public VulkanBuffer {
  public:
    explicit VulkanBakedPrimitiveInstanceBuffer(
        VulkanRenderer& renderer, std::span<const BakedPrimitiveInstance> primitive_instances
    );

  private:
    auto load_to_gpu() -> void;

    std::span<const BakedPrimitiveInstance> m_baked_primitive_instances = {};
};
