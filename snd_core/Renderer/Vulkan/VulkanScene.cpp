#include "VulkanScene.hpp"

#include "snd_core/Resources/Scene/Scene.hpp"

VulkanScene::VulkanScene(VulkanRenderer& device, const Scene& scene)
    : m_scene(scene), m_renderer(device) {
}
auto VulkanScene::load_vertices_to_gpu_memory() -> void {
    vk::DeviceSize vertex_size = sizeof(Vertex) * m_scene.get_data().vertices.size();
    vk::DeviceSize index_size = sizeof(uint32_t) * m_scene.get_data().indices.size();

    auto buffer_create_info = vk::BufferCreateInfo {
        .size = vertex_size + index_size,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
    };

    auto allocation_create_info = vma::AllocationCreateInfo {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite |
                 vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead |
                 vma::AllocationCreateFlagBits::eMapped,
        .usage = vma::MemoryUsage::eAuto,
    };

    // TODO: use a staging buffer on gpus that don't support resizeable bar
    m_vertex_and_index_buffer = m_renderer.get_device().get_allocator().createBuffer(
        buffer_create_info, allocation_create_info
    );
    const vma::raii::Allocation& allocation = m_vertex_and_index_buffer.getAllocation();

    allocation.copyFromMemory(m_scene.get_data().vertices.data(), 0, vertex_size);
    allocation.copyFromMemory(m_scene.get_data().indices.data(), vertex_size, index_size);
}

auto VulkanScene::load_images_to_gpu_memory() -> void {
    for (const auto& image : m_scene.get_data().images) {
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

        vma::raii::Image gpu_image = m_renderer.get_device().get_allocator().createImage(
            image_create_info, allocation_create_info
        );

        // even more assumptions about the image is made here
        auto image_view_create_info = vk::ImageViewCreateInfo {
            .image = gpu_image,
            .viewType = vk::ImageViewType::e2D,
            .format = vk::Format::eR8G8B8A8Srgb,
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1,
            },
        };

        vk::raii::ImageView image_view =
            m_renderer.get_device().inner().createImageView(image_view_create_info);

        m_texture_images.emplace_back(std::move(gpu_image));
        m_texture_image_views.emplace_back(std::move(image_view));
    }

    // now prepare the commands that are necessary to transition the image to an usable form
    auto commands = [&](vk::CommandBuffer& command_buffer) -> void {
        for (size_t i = 0; i < m_texture_images.size(); i++) {
            const vk::raii::ImageView& image_view = m_texture_image_views[i];
            const vma::raii::Image& image = m_texture_images[i];

            auto image_barrier = vk::ImageMemoryBarrier2 {
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .image = image,
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

            // todo: support mips
            // std::vector<vk::BufferImageCopy> copy_regions = {};

            auto image_barrier_read = vk::ImageMemoryBarrier2 {
                .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
                .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eReadOnlyOptimal,
                .image = image,
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
        }
    };

    m_scene_create_commands.emplace_back(commands);
}

auto VulkanScene::load_samplers() -> void {
    for (const auto& sampler : m_scene.get_data().samplers) {
        // gltf values are same as the vulkan values for these properties
        auto sampler_create_info = vk::SamplerCreateInfo {
            .magFilter = static_cast<vk::Filter>(sampler.mag_filter),
            .minFilter = static_cast<vk::Filter>(sampler.min_filter),
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .anisotropyEnable = vk::True,
            .maxAnisotropy = 8.0f,
            .maxLod = 1
        };

        vk::raii::Sampler vulkan_sampler =
            m_renderer.get_device().inner().createSampler(sampler_create_info);

        m_samplers.emplace_back(std::move(vulkan_sampler));
    }
}
