#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "../../DeletionQueue.hpp"
#include "VulkanDevice.hpp"

class CommandBuffer {
  public:
    CommandBuffer() = default;
    CommandBuffer(VulkanDevice& device);

    auto inner() -> vk::raii::CommandPool&;
    auto push_deletion_function(std::function<void()>&& function) -> void;
    auto run(std::function<void(vk::raii::CommandBuffer& command_buffer)> commands) -> void;

    bool is_initialized = false;

    ~CommandBuffer();

  private:
    vk::raii::CommandPool m_command_pool = nullptr;
    DeletionQueue m_deletion_queue = {};
};