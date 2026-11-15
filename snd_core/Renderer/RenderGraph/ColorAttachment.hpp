#pragma once
#include <vulkan/vulkan.hpp>

struct ColorAttachment {
    float size_x = 0.0f;
    float size_y = 0.0f;
    vk::Format format = vk::Format::eUndefined;
};
