#pragma once
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

class VulkanRenderer;

class VulkanVertexBuffer : public VulkanBuffer {
  public:
    explicit VulkanVertexBuffer(
        VulkanRenderer& renderer, std::span<const Vertex> vertices, std::span<const uint32_t> indices
    );

  private:
    auto load_to_gpu() -> void;

    std::span<const Vertex> m_vertices = {};
    std::span<const uint32_t> m_indices = {};
};
