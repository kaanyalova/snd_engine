#pragma once
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"

class VulkanRenderer;

struct VulkanSceneData {
    vk::DeviceAddress vertices;
    vk::DeviceAddress scene_transforms;
    vk::DeviceAddress primitive_instances;
    vk::DeviceAddress primitives;
    vk::DeviceAddress materials;
};

class VulkanSceneBuffer : public VulkanBuffer {
  public:
    explicit VulkanSceneBuffer(VulkanRenderer& renderer, const VulkanSceneData& scene_data);

  private:
    auto load_to_gpu() -> void;

    VulkanSceneData m_scene_data = {};
};
