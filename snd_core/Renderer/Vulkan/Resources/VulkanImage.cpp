#include "VulkanImage.hpp"

#include "../VulkanRenderer.hpp"
VulkanImage::VulkanImage(VulkanRenderer& renderer, VulkanScene& scene, const ImageView& image) : m_renderer(renderer) {
    allocate_image(image);
    allocate_image_buffer(image);
    copy_image_data_to_buffer(image);
    copy_buffer_to_image(scene);
}

auto VulkanImage::allocate_image(const ImageView& image) -> void {
    // todo: the image type is probably rgba8, and the gpu probably supports it but its better
    // to convert it to a format that i am sure that the gpu supports and the image format is
    auto image_create_info = vk::ImageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
        .extent =
            vk::Extent3D {
                .width = image.width,
                .height = image.height,
                .depth = 1,
            },
        .mipLevels = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    vma::AllocationCreateInfo allocation_create_info = {
        .usage = vma::MemoryUsage::eAuto,
    };

    m_image = m_renderer.get_device().get_allocator().createImage(image_create_info, allocation_create_info);

    // even more assumptions about the image is made here
    auto image_view_create_info = vk::ImageViewCreateInfo {
        .image = m_image,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    m_image_view = m_renderer.get_device().inner().createImageView(image_view_create_info);
}

auto VulkanImage::copy_buffer_to_image(VulkanScene& scene) -> void {
    auto command = [&](const vk::CommandBuffer& command_buffer) {
        auto image_barrier = vk::ImageMemoryBarrier2 {
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .image = m_image,
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1,
            },
        };

        auto barrier_dependency_info = vk::DependencyInfo {
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_barrier,
        };

        command_buffer.pipelineBarrier2(barrier_dependency_info);
        command_buffer.copyBufferToImage(m_buffer, m_image, vk::ImageLayout::eTransferDstOptimal, {});

        // todo: support mips
        // std::vector<vk::BufferImageCopy> copy_regions = {};

        auto image_barrier_read = vk::ImageMemoryBarrier2 {
            .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = vk::ImageLayout::eReadOnlyOptimal,
            .image = m_image,
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1,
            },
        };

        auto barrier_dependency_read_info = vk::DependencyInfo {
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_barrier_read,
        };

        command_buffer.pipelineBarrier2(barrier_dependency_info);
    };
    scene.push_create_command(command);
}

auto VulkanImage::allocate_image_buffer(const ImageView& image) -> void {
    auto buffer_create_info = vk::BufferCreateInfo {
        .size = image.data.size(),
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
    };

    auto allocation_create_info = vma::AllocationCreateInfo {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
        .usage = vma::MemoryUsage::eAuto,
    };

    m_buffer = m_renderer.get_device().get_allocator().createBuffer(buffer_create_info, allocation_create_info);
}

auto VulkanImage::copy_image_data_to_buffer(const ImageView& image) -> void {
    void* buffer = m_buffer.getAllocation().map();
    std::memcpy(buffer, image.data.data(), image.data.size());
}

VulkanImage::~VulkanImage() {
    m_renderer.mark_image_slot_for_deletion(m_bound_slot);
}
