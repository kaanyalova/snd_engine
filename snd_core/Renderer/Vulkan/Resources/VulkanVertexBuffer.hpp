#pragma once
#include <span>
#include <vk_mem_alloc_raii.hpp>

#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

class VulkanVertexBuffer {
  public:
    explicit VulkanVertexBuffer(
        VulkanRenderer& renderer, std::span<const Vertex> vertices, std::span<const uint32_t> indices
    );
    auto get_device_address() const -> uint64_t { return m_buffer_device_address; }

  private:
    auto load_to_gpu() -> void;
    auto fetch_device_address() -> void;

    VulkanRenderer& m_renderer;

    std::span<const Vertex> m_vertices = {};
    std::span<const uint32_t> m_indices = {};

    vma::raii::Buffer m_buffer = nullptr;
    uint64_t m_buffer_device_address = 0;
};
