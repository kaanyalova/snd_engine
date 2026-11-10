#include "VulkanSceneBuffer.hpp"

VulkanSceneBuffer::VulkanSceneBuffer(VulkanRenderer& renderer, const VulkanSceneData& scene_data)
    : VulkanBuffer(renderer), m_scene_data(scene_data) {
    load_to_gpu();
}

auto VulkanSceneBuffer::load_to_gpu() -> void {
    create_mapped_buffer(sizeof(VulkanSceneData), vk::BufferUsageFlagBits::eStorageBuffer);
    copy_to_buffer(&m_scene_data, 0, sizeof(VulkanSceneData));
}
