#include "VulkanMaterialBuffer.hpp"

VulkanMaterialBuffer::VulkanMaterialBuffer(VulkanRenderer& renderer, std::span<const MaterialData> materials)
    : VulkanBuffer(renderer), m_materials(materials) {
    load_to_gpu();
}

auto VulkanMaterialBuffer::load_to_gpu() -> void {
    vk::DeviceSize material_size = sizeof(MaterialData) * m_materials.size();

    create_mapped_buffer(material_size, vk::BufferUsageFlagBits::eStorageBuffer);

    if (m_materials.size() > 0) {
        copy_to_buffer(m_materials.data(), 0, material_size);
    }
}
