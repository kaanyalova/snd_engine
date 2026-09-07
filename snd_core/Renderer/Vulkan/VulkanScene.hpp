#pragma once

#include "VulkanRenderer.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"

class Scene;

using CommandBufferRecordLambda = std::function<void(vk::CommandBuffer& command_buffer)>;

class VulkanScene {
  public:
    explicit VulkanScene(VulkanRenderer& device, const Scene& scene);

  private:
    const Scene& m_scene;
    VulkanRenderer& m_renderer;

    // vertices
    vma::raii::Buffer m_vertex_and_index_buffer = nullptr;

    // textures
    std::vector<vma::raii::Image> m_texture_images;
    std::vector<vk::raii::ImageView> m_texture_image_views;

    // samplers
    std::vector<vk::raii::Sampler> m_samplers;

    std::vector<CommandBufferRecordLambda> m_scene_create_commands;

    auto load_vertices_to_gpu_memory() -> void;
    auto load_images_to_gpu_memory() -> void;
    auto load_samplers() -> void;
};