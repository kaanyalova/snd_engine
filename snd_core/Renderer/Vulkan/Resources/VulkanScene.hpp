#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "VulkanImage.hpp"
#include "VulkanSampler.hpp"
#include "VulkanVertexBuffer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"
#include "vk_mem_alloc_raii.hpp"
#include "vulkan/vulkan.hpp"

class Scene;
class VulkanRenderer;

using CommandBufferRecordLambda = std::function<void(vk::raii::CommandBuffer& command_buffer)>;

class VulkanScene {
  public:
    VulkanScene() = delete;
    explicit VulkanScene(VulkanRenderer& renderer, Scene& scene);
    auto run_create_commands(vk::raii::CommandBuffer& command_buffer) -> void;
    auto push_create_command(CommandBufferRecordLambda&& record) -> void;

  private:
    const Scene& m_scene;
    VulkanRenderer& m_renderer;

    std::vector<std::unique_ptr<VulkanSampler>> m_samplers = {};
    // the vulkan images are heap allocated because the callbacks inside hold references to themselves
    // which gets invalidated when the vector reallocates
    std::vector<std::unique_ptr<VulkanImage>> m_images = {};
    std::unique_ptr<VulkanVertexBuffer> m_vertex_buffer = nullptr;

    vma::raii::Buffer m_scene_indirect_draw_commands_buffer = nullptr;

    std::vector<CommandBufferRecordLambda> m_scene_create_commands = {};

    auto build_indirect_draw_commands() -> void;

    auto walk_scene_tree() -> void;

    static auto model_range_to_draw_command(const ModelRange& model_range) -> vk::DrawIndexedIndirectCommand;

    auto bind_images_to_descriptors() -> void;
    auto load_vertices_to_gpu() -> void;
    auto load_images_to_gpu() -> void;
    auto load_samplers_to_gpu() -> void;
    auto bind_samplers_to_descriptors() -> void;
};