#pragma once
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

class VulkanRenderer;

class VulkanMaterialBuffer : public VulkanBuffer {
  public:
    explicit VulkanMaterialBuffer(VulkanRenderer& renderer, std::span<const MaterialData> materials);

  private:
    auto load_to_gpu() -> void;

    std::span<const MaterialData> m_materials = {};
};
