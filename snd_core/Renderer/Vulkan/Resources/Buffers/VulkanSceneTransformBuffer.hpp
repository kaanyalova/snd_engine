#pragma once
#include <glm/glm.hpp>
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"

class VulkanRenderer;

class VulkanSceneTransformBuffer : public VulkanBuffer {
  public:
    explicit VulkanSceneTransformBuffer(VulkanRenderer& renderer, std::span<const glm::mat4x4> scene_transforms);

    auto set_transform_of(size_t index, const glm::mat4x4& transform) -> void;

  private:
    auto load_to_gpu() -> void;

    std::span<const glm::mat4x4> m_scene_transforms = {};
};
