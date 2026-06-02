#include "ImageViewInfo.hpp"

#include "vulkan/vulkan.hpp"

auto ImageViewInfo::eR8G8B8A8Srgb_color() -> ImageViewInfo {
    return ImageViewInfo {
        .format = vk::Format::eR8G8B8A8Srgb,
        .aspect_flags = vk::ImageAspectFlagBits::eColor,
    };
}

auto ImageViewInfo::depth(vk::Format depth_format) -> ImageViewInfo {
    return ImageViewInfo {
        .format = depth_format,
        .aspect_flags = vk::ImageAspectFlagBits::eDepth,
    };
}
