#pragma once
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "snd_core/Image/Image.hpp"

class VulkanRenderer;
class VulkanScene;

/**
 * An image that's allocated on the GPU and that's bound to one of the slots on the descriptor set
 * when the image is deleted it marks itself so that the slot the image is bound to can be reused
 */
class VulkanImage {
  public:
    VulkanImage(VulkanRenderer& renderer, VulkanScene& scene, const ImageView& image);
    auto inner() const -> const vk::Image& { return m_image; }
    auto inner_view() const -> const vk::ImageView& { return m_image_view; }

    // this is to be called when the image is loaded by the scene and it is bound to the descriptor
    auto set_slot(uint32_t slot) -> void { m_bound_slot = slot; }

    ~VulkanImage();

  private:
    VulkanRenderer& m_renderer;

    vma::raii::Image m_image = nullptr;
    vk::raii::ImageView m_image_view = nullptr;
    vma::raii::Buffer m_buffer = nullptr;

    int32_t m_bound_slot = -1;

    auto allocate_image(const ImageView& image) -> void;
    auto allocate_image_buffer(const ImageView& image) -> void;
    auto copy_image_data_to_buffer(const ImageView& image) -> void;
    auto copy_buffer_to_image(VulkanScene& scene) -> void;
};
