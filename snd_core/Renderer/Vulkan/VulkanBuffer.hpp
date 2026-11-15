#pragma once
#include <span>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

class VulkanRenderer;

class VulkanBuffer {
  public:
    VulkanBuffer(VulkanRenderer& renderer, vk::DeviceSize size, vk::BufferUsageFlags usage);

    auto get_device_address() const -> vk::DeviceAddress { return m_device_address; }
    auto get_buffer() const -> const vk::Buffer& { return m_buffer; }
    auto get_size() const -> vk::DeviceSize { return m_size; }

    auto copy_to_buffer(const void* source, vk::DeviceSize offset, vk::DeviceSize size) -> void;

    template <typename Container>
    auto upload(const Container& data, vk::DeviceSize offset = 0) -> void {
        if (!data.empty()) {
            copy_to_buffer(data.data(), offset, data.size() * sizeof(*data.data()));
        }
    }

  private:
    auto fetch_device_address() -> void;

    VulkanRenderer& m_renderer;
    vma::raii::Buffer m_buffer = nullptr;
    vk::DeviceAddress m_device_address = 0;
    vk::DeviceSize m_size = 0;
};
