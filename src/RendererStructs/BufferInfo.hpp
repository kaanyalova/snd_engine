#pragma once

#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

struct BufferInfo {
    vk::BufferUsageFlags buffer_usage;
    vma::MemoryUsage allocation_usage;
    vma::AllocationCreateFlags allocation_flags;

    static auto vertex_staging_buffer() -> BufferInfo;
    static auto vertex_device_buffer() -> BufferInfo;

    static auto index_staging_buffer() -> BufferInfo;
    static auto index_device_buffer() -> BufferInfo;

    static auto default_staging_buffer() -> BufferInfo;

    static auto uniform_buffer() -> BufferInfo;

    static auto image_staging_buffer() -> BufferInfo;
};