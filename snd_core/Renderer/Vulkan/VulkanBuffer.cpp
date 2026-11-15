#include "VulkanBuffer.hpp"

#include <vk_mem_alloc.hpp>

#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"

VulkanBuffer::VulkanBuffer(VulkanRenderer& renderer, vk::DeviceSize size, vk::BufferUsageFlags usage)
    : m_renderer(renderer), m_size(size) {
    auto buffer_create_info = vk::BufferCreateInfo {
        .size = size,
        .usage = usage | vk::BufferUsageFlagBits::eShaderDeviceAddress,
    };

    auto allocation_create_info = vma::AllocationCreateInfo {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite
            | vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead
            | vma::AllocationCreateFlagBits::eMapped,
        .usage = vma::MemoryUsage::eAuto,
    };

    m_buffer = m_renderer.get_device().get_allocator().createBuffer(buffer_create_info, allocation_create_info);
    fetch_device_address();
}

auto VulkanBuffer::copy_to_buffer(const void* source, vk::DeviceSize offset, vk::DeviceSize size) -> void {
    m_buffer.getAllocation().copyFromMemory(source, offset, size);
}

auto VulkanBuffer::fetch_device_address() -> void {
    m_device_address = m_renderer.get_device().get_address(m_buffer);
}
