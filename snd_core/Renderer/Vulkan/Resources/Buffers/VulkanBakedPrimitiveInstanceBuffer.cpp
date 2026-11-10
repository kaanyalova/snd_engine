#include "VulkanBakedPrimitiveInstanceBuffer.hpp"

VulkanBakedPrimitiveInstanceBuffer::VulkanBakedPrimitiveInstanceBuffer(
    VulkanRenderer& renderer, std::span<const BakedPrimitiveInstance> primitive_instances
)
    : VulkanBuffer(renderer), m_baked_primitive_instances(primitive_instances) {
    load_to_gpu();
}

auto VulkanBakedPrimitiveInstanceBuffer::load_to_gpu() -> void {
    vk::DeviceSize primitive_instance_size = sizeof(BakedPrimitiveInstance) * m_baked_primitive_instances.size();

    create_mapped_buffer(primitive_instance_size, vk::BufferUsageFlagBits::eStorageBuffer);

    if (m_baked_primitive_instances.size() > 0) {
        copy_to_buffer(m_baked_primitive_instances.data(), 0, primitive_instance_size);
    }
}
