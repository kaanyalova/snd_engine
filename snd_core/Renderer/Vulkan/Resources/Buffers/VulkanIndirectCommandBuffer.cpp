#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanIndirectCommandBuffer.hpp"

VulkanIndirectCommandBuffer::VulkanIndirectCommandBuffer(
    VulkanRenderer& renderer, std::span<const vk::DrawIndexedIndirectCommand> commands
)
    : VulkanBuffer(renderer) {
    load_to_gpu();
}

auto VulkanIndirectCommandBuffer::load_to_gpu() -> void {
    vk::DeviceSize size = sizeof(vk::DrawIndexedIndirectCommand) * m_commands.size();

    create_mapped_buffer(
        size,
        vk::BufferUsageFlagBits::eTransferDst
            | vk::BufferUsageFlagBits::eStorageBuffer
            | vk::BufferUsageFlagBits::eIndirectBuffer
    );

    if (m_commands.size() > 0) {
        copy_to_buffer(m_commands.data(), 0, size);
    }
}