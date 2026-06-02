#include "ImageInfo.hpp"

#include "vulkan/vulkan.hpp"

auto ImageInfo::r8g8b8a8_srgb() -> ImageInfo {
    return ImageInfo {
        .format = vk::Format::eR8G8B8A8Srgb,
        .tiling = vk::ImageTiling::eOptimal,
        .usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
    };
}

auto ImageInfo::depth_format(vk::Format image_format) -> ImageInfo {
    return ImageInfo {
        .format = image_format,
        .tiling = vk::ImageTiling::eOptimal,
        .usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .memory_flags = vk::MemoryPropertyFlagBits::eDeviceLocal,
    };
}
