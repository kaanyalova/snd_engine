#include "snd_core/Renderer/Vulkan/VulkanCommandBuffer.hpp"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device) {
    is_initialized = true;
}

auto VulkanCommandBuffer::inner() -> vk::raii::CommandPool& {
    return m_command_pool;
}

auto VulkanCommandBuffer::push_deletion_function(std::function<void()>&& function) -> void {
    m_deletion_queue.push_function(std::move(function));
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    m_deletion_queue.flush();
}