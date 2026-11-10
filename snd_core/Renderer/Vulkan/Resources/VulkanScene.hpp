#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Buffers/VulkanBakedPrimitiveInstanceBuffer.hpp"
#include "Buffers/VulkanIndirectCommandBuffer.hpp"
#include "Buffers/VulkanMaterialBuffer.hpp"
#include "Buffers/VulkanSceneBuffer.hpp"
#include "Buffers/VulkanSceneTransformBuffer.hpp"
#include "Buffers/VulkanVertexBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanSampler.hpp"
#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanPrimitiveBuffer.hpp"
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
    auto build_indirect_draw_commands() -> void;
    auto build_and_get_indirect_draw_commands() -> const std::vector<vk::DrawIndexedIndirectCommand>& {
        build_indirect_draw_commands();
        return m_draw_indexed_indirect_commands;
    }

  private:
    const Scene& m_scene;
    VulkanRenderer& m_renderer;

    std::vector<std::unique_ptr<VulkanSampler>> m_samplers = {};
    // the vulkan images are heap allocated because the callbacks inside hold references to themselves
    // which gets invalidated when the vector reallocates
    std::vector<std::unique_ptr<VulkanImage>> m_images = {};

    std::unique_ptr<VulkanVertexBuffer> m_vertex_buffer = nullptr;
    std::unique_ptr<VulkanMaterialBuffer> m_material_buffer = nullptr;
    std::unique_ptr<VulkanPrimitiveBuffer> m_primitives_buffer = nullptr;
    std::unique_ptr<VulkanBakedPrimitiveInstanceBuffer> m_primitive_instance_buffer = nullptr;
    std::unique_ptr<VulkanSceneTransformBuffer> m_scene_transform_buffer = nullptr;
    std::unique_ptr<VulkanIndirectCommandBuffer> m_indirect_command_buffer = nullptr;

    std::unique_ptr<VulkanSceneBuffer> m_scene_buffer = nullptr;

    std::vector<CommandBufferRecordLambda> m_scene_create_commands = {};
    std::vector<vk::DrawIndexedIndirectCommand> m_draw_indexed_indirect_commands = {};

    auto load_scene_buffers() -> void;
    auto get_gpu_scene_data() -> VulkanSceneData;

    static auto model_range_to_draw_command(const ModelRange& model_range) -> vk::DrawIndexedIndirectCommand;

    auto bind_images_to_descriptors() -> void;
    auto load_images_to_gpu() -> void;
    auto load_samplers_to_gpu() -> void;
    auto bind_samplers_to_descriptors() -> void;
};