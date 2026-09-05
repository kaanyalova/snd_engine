#pragma once

#include <cstdint>
#include <vector>

enum class ImageFormat : uint8_t {
    Undefined,
    Rgba8,
    Rgb8,
};

struct Image {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> data = {};
    ImageFormat format = ImageFormat::Undefined;
};