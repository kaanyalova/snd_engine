#include "BufferInfo.hpp"

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