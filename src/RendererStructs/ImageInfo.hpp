#pragma once
#include <vulkan/vulkan.hpp>

struct ImageInfo {
    vk::Format format;
    vk::ImageTiling tiling;
    vk::ImageUsageFlags usage_flags;
    vk::MemoryPropertyFlags memory_flags;

    static auto r8g8b8a8_srgb() -> ImageInfo;
    static auto depth_format(vk::Format image_format) -> ImageInfo;
};