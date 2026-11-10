#include "VulkanPrimitiveBuffer.hpp"

VulkanPrimitiveBuffer::VulkanPrimitiveBuffer(VulkanRenderer& renderer, std::span<const Primitive> primitives)
    : VulkanBuffer(renderer), m_primitives(primitives) {
    load_to_gpu();
}

auto VulkanPrimitiveBuffer::load_to_gpu() -> void {
    vk::DeviceSize primitive_size = sizeof(Primitive) * m_primitives.size();

    create_mapped_buffer(primitive_size, vk::BufferUsageFlagBits::eStorageBuffer);

    if (m_primitives.size() > 0) {
        copy_to_buffer(m_primitives.data(), 0, primitive_size);
    }
}
