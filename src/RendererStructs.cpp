#include "RendererStructs.hpp"

#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan.hpp"

auto BufferInfo::vertex_staging_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage =
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        .allocation_usage = vma::MemoryUsage::eAuto,
        .allocation_flags = vma::AllocationCreateFlagBits::eMapped |
                            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
    };
}

auto BufferInfo::vertex_device_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage =
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .allocation_usage = vma::MemoryUsage::eAutoPreferDevice,
    };
}

auto BufferInfo::index_staging_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage =
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        .allocation_usage = vma::MemoryUsage::eAuto,
        .allocation_flags = vma::AllocationCreateFlagBits::eMapped |
                            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
    };
}

auto BufferInfo::index_device_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage =
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .allocation_usage = vma::MemoryUsage::eAutoPreferDevice,
    };
}

auto BufferInfo::uniform_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage = vk::BufferUsageFlagBits::eUniformBuffer,

        .allocation_usage = vma::MemoryUsage::eAuto,  // TODO: should this be auto?
        .allocation_flags = vma::AllocationCreateFlagBits::eMapped |
                            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
    };
}

auto BufferInfo::default_staging_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage = vk::BufferUsageFlagBits::eTransferSrc,
        .allocation_usage = vma::MemoryUsage::eAuto,
        .allocation_flags = vma::AllocationCreateFlagBits::eMapped |
                            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
    };
}

auto BufferInfo::image_staging_buffer() -> BufferInfo {
    return BufferInfo {
        .buffer_usage = vk::BufferUsageFlagBits::eTransferSrc,
        .allocation_usage = vma::MemoryUsage::eAuto,
        .allocation_flags = vma::AllocationCreateFlagBits::eMapped |
                            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,

    };
}

auto ImageInfo::r8g8b8a8Srgb() -> ImageInfo {
    return ImageInfo {
        .format = vk::Format::eR8G8B8A8Srgb,
        .tiling = vk::ImageTiling::eOptimal,
        .usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
    };
}

auto ImageTransitionInfo::dst_optimal_to_shader_optimal() -> ImageTransitionInfo {
    return ImageTransitionInfo {
        .from = vk::ImageLayout::eTransferDstOptimal,
        .to = vk::ImageLayout::eShaderReadOnlyOptimal,
    };
}
