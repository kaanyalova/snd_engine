#include "VulkanScene.hpp"

#include <ranges>

#include "snd_core/Resources/Scene/Scene.hpp"

VulkanScene::VulkanScene(VulkanRenderer& device, Scene& scene) : m_scene(scene), m_renderer(device) {
    load_images_to_gpu();
    load_vertices_to_gpu();

    load_samplers_to_gpu();
    bind_images_to_descriptors();
    bind_samplers_to_descriptors();
    scene.mark_loaded_on_gpu(this);
}

auto VulkanScene::push_create_command(CommandBufferRecordLambda&& record) -> void {
    m_scene_create_commands.emplace_back(std::move(record));
}

auto VulkanScene::bind_images_to_descriptors() -> void {
    auto image_infos = m_images
        | std::ranges::views::transform([](const std::unique_ptr<VulkanImage>& vulkan_image) {
                           return vulkan_image.get();
                       })
        | std::ranges::views::transform([](const VulkanImage* image) {
                           return vk::DescriptorImageInfo {
                               .sampler = nullptr,
                               .imageView = image->inner_view(),
                               .imageLayout = vk::ImageLayout::eReadOnlyOptimal,
                           };
                       })
        | std::ranges::to<std::vector>();

    auto slot_start = m_renderer.get_next_free_image_slots(m_images.size());

    auto descriptor_write = vk::WriteDescriptorSet {
        .dstSet = m_renderer.get_renderer_descriptor_set(),
        .dstBinding = static_cast<uint32_t>(VulkanRenderer::DescriptorSetBindings::SampledImages),
        .dstArrayElement = slot_start,
        .descriptorCount = static_cast<uint32_t>(m_images.size()),
        .descriptorType = vk::DescriptorType::eSampledImage,
        .pImageInfo = image_infos.data(),
    };

    m_renderer.get_device().inner().updateDescriptorSets(descriptor_write, 0);

    for (size_t i = 0; i < m_images.size(); i++) {
        m_images[i]->set_slot(slot_start + i);
    }
}

auto VulkanScene::load_vertices_to_gpu() -> void {
    m_vertex_buffer =
        std::make_unique<VulkanVertexBuffer>(m_renderer, m_scene.get_data().vertices, m_scene.get_data().indices);
}

auto VulkanScene::load_images_to_gpu() -> void {
    for (auto& image : m_scene.get_data().images) {
        ImageView view = image.view_of(m_scene.get_data().image_data);
        m_images.emplace_back(std::make_unique<VulkanImage>(m_renderer, *this, view));
    }
}

auto VulkanScene::load_samplers_to_gpu() -> void {
    for (auto& sampler : m_scene.get_data().samplers) {
        m_samplers.emplace_back(std::make_unique<VulkanSampler>(m_renderer, sampler));
    }
}

auto VulkanScene::bind_samplers_to_descriptors() -> void {
    std::vector<vk::DescriptorImageInfo> sampler_image_infos = m_samplers
        | std::ranges::views::transform([](const std::unique_ptr<VulkanSampler>& sampler) { return sampler.get(); })
        | std::ranges::views::transform([](const VulkanSampler* sampler) {
                                                                   return vk::DescriptorImageInfo {
                                                                       .sampler = sampler->inner(),
                                                                       .imageView = nullptr,
                                                                       .imageLayout = vk::ImageLayout::eReadOnlyOptimal,
                                                                   };
                                                               })
        | std::ranges::to<std::vector>();

    uint32_t slot_start = m_renderer.get_next_free_sampler_slots(m_samplers.size());

    auto descriptor_write = vk::WriteDescriptorSet {
        .dstSet = m_renderer.get_renderer_descriptor_set(),
        .dstBinding = static_cast<uint32_t>(VulkanRenderer::DescriptorSetBindings::SampledImages),
        .dstArrayElement = slot_start,
        .descriptorCount = static_cast<uint32_t>(m_samplers.size()),
        .descriptorType = vk::DescriptorType::eSampledImage,
        .pImageInfo = sampler_image_infos.data(),
    };

    m_renderer.get_device().inner().updateDescriptorSets(descriptor_write, 0);

    for (size_t i = 0; i < m_samplers.size(); i++) {
        m_samplers[i]->set_slot(slot_start + i);
    }
}