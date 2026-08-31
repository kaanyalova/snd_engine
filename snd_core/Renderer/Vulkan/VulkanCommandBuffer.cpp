#include "snd_core/Renderer/Vulkan/VulkanCommandBuffer.hpp"

CommandBuffer::CommandBuffer(VulkanDevice& device) {
    is_initialized = true;
}

auto CommandBuffer::inner() -> vk::raii::CommandPool& {
    return m_command_pool;
}

auto CommandBuffer::push_deletion_function(std::function<void()>&& function) -> void {
    m_deletion_queue.push_function(std::move(function));
}

CommandBuffer::~CommandBuffer() {
    m_deletion_queue.flush();
}