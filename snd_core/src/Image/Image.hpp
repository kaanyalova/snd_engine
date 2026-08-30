#pragma once

#include <cstdint>
#include <vector>

enum class ImageFormat : uint8_t {
    Undefined,
    Rgba8,
    Rgb8,
};

struct Image {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;
    ImageFormat format = ImageFormat::Undefined;
};