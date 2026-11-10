#include "VulkanSceneTransformBuffer.hpp"

VulkanSceneTransformBuffer::VulkanSceneTransformBuffer(
    VulkanRenderer& renderer, std::span<const glm::mat4x4> scene_transforms
)
    : VulkanBuffer(renderer), m_scene_transforms(scene_transforms) {
    load_to_gpu();
}

auto VulkanSceneTransformBuffer::set_transform_of(size_t index, const glm::mat4x4& transform) -> void {
    copy_to_buffer(&transform, index * sizeof(glm::mat4x4), sizeof(glm::mat4x4));
}

auto VulkanSceneTransformBuffer::load_to_gpu() -> void {
    vk::DeviceSize scene_transform_size = sizeof(glm::mat4x4) * m_scene_transforms.size();

    create_mapped_buffer(scene_transform_size, vk::BufferUsageFlagBits::eStorageBuffer);

    if (m_scene_transforms.size() > 0) {
        copy_to_buffer(m_scene_transforms.data(), 0, scene_transform_size);
    }
}
