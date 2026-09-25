#pragma once

#include "../VulkanRenderer.hpp"
#include "VulkanSampler.hpp"
#include "VulkanVertexBuffer.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"

class Scene;

using CommandBufferRecordLambda = std::function<void(vk::CommandBuffer& command_buffer)>;

class VulkanScene {
  public:
    explicit VulkanScene(VulkanRenderer& device, Scene& scene);

  private:
    const Scene& m_scene;
    VulkanRenderer& m_renderer;

    std::vector<std::unique_ptr<VulkanSampler>> m_samplers = {};
    // the vulkan images are heap allocated because the callbacks inside hold references to themselves
    // which gets invalidated when the vector reallocates
    std::vector<std::unique_ptr<VulkanImage>> m_images = {};
    std::unique_ptr<VulkanVertexBuffer> m_vertex_buffer = nullptr;

    std::vector<CommandBufferRecordLambda> m_scene_create_commands = {};

    auto bind_images_to_descriptors() -> void;
    auto load_vertices_to_gpu() -> void;
    auto load_images_to_gpu() -> void;
    auto load_samplers_to_gpu() -> void;
    auto bind_samplers_to_descriptors() -> void;

    auto push_create_command(CommandBufferRecordLambda&& record) -> void;
};