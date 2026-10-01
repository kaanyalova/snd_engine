#include "VulkanVertexBuffer.hpp"

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

VulkanVertexBuffer::VulkanVertexBuffer(
    VulkanRenderer& renderer, std::span<const Vertex> vertices, std::span<const uint32_t> indices
)
    : m_renderer(renderer), m_vertices(vertices), m_indices(indices) {
    load_to_gpu();
    fetch_device_address();
}

auto VulkanVertexBuffer::load_to_gpu() -> void {
    vk::DeviceSize vertex_size = sizeof(Vertex) * m_vertices.size();
    vk::DeviceSize index_size = sizeof(uint32_t) * m_indices.size();

    auto buffer_create_info = vk::BufferCreateInfo {
        .size = vertex_size + index_size,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
    };

    auto allocation_create_info = vma::AllocationCreateInfo {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
            | vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead
            | vma::AllocationCreateFlagBits::eMapped,
        .usage = vma::MemoryUsage::eAuto,
    };

    // TODO: use a staging buffer on gpus that don't support resizeable bar
    m_buffer = m_renderer.get_device().get_allocator().createBuffer(buffer_create_info, allocation_create_info);
    const vma::raii::Allocation& allocation = m_buffer.getAllocation();

    if (m_vertices.size() > 0) {
        allocation.copyFromMemory(m_vertices.data(), 0, vertex_size);
    }
    
    if (m_indices.size() > 0) {
        allocation.copyFromMemory(m_indices.data(), vertex_size, index_size);
    }
}

auto VulkanVertexBuffer::fetch_device_address() -> void {
    auto device_address_info = vk::BufferDeviceAddressInfo {
        .buffer = m_buffer,
    };

    m_buffer_device_address = m_renderer.get_device().inner().getBufferAddress(device_address_info);
}