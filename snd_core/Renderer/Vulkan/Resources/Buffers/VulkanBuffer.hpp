#pragma once
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

class VulkanRenderer;

class VulkanBuffer {
  public:
    auto get_device_address() const -> vk::DeviceAddress { return m_device_address; }
    auto get_buffer() const -> const vk::Buffer& { return m_buffer; }

  protected:
    explicit VulkanBuffer(VulkanRenderer& renderer);

    auto create_mapped_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage) -> void;
    auto copy_to_buffer(const void* source, vk::DeviceSize offset, vk::DeviceSize size) -> void;
    auto fetch_device_address() -> void;

    VulkanRenderer& m_renderer;
    vma::raii::Buffer m_buffer = nullptr;
    vk::DeviceAddress m_device_address = 0;
};
