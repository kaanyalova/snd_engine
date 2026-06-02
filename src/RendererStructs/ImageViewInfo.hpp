#pragma once

#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"

struct ImageViewInfo {
    vk::Format format;
    vk::ImageAspectFlags aspect_flags;

    static auto eR8G8B8A8Srgb_color() -> ImageViewInfo;
    static auto depth(vk::Format format) -> ImageViewInfo;
};