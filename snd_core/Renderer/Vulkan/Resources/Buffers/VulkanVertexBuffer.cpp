#include "VulkanVertexBuffer.hpp"

VulkanVertexBuffer::VulkanVertexBuffer(
    VulkanRenderer& renderer, std::span<const Vertex> vertices, std::span<const uint32_t> indices
)
    : VulkanBuffer(renderer), m_vertices(vertices), m_indices(indices) {
    load_to_gpu();
}

auto VulkanVertexBuffer::load_to_gpu() -> void {
    vk::DeviceSize vertex_size = sizeof(Vertex) * m_vertices.size();
    vk::DeviceSize index_size = sizeof(uint32_t) * m_indices.size();

    create_mapped_buffer(
        vertex_size + index_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer
    );

    if (m_vertices.size() > 0) {
        copy_to_buffer(m_vertices.data(), 0, vertex_size);
    }

    if (m_indices.size() > 0) {
        copy_to_buffer(m_indices.data(), vertex_size, index_size);
    }
}
