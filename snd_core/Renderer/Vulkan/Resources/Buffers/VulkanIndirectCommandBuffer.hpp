#pragma once
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

class VulkanRenderer;
#pragma once
#include <span>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

class VulkanRenderer;

class VulkanIndirectCommandBuffer : public VulkanBuffer {
  public:
    explicit VulkanIndirectCommandBuffer(
        VulkanRenderer& renderer, std::span<const vk::DrawIndexedIndirectCommand> materials
    );

  private:
    auto load_to_gpu() -> void;

    std::span<const vk::DrawIndexedIndirectCommand> m_commands = {};
};
