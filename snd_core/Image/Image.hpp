#pragma once

#include <cstdint>
#include <span>
#include <vector>

enum class ImageFormat : uint8_t {
    Undefined,
    Rgba8,
    Rgb8,
};

// just like an image but does not own the data
struct ImageView {
    uint32_t width = 0;
    uint32_t height = 0;
    std::span<const uint8_t> data = {};
    ImageFormat format = ImageFormat::Undefined;
};

struct Image {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> data = {};
    ImageFormat format = ImageFormat::Undefined;

    auto view() -> ImageView;
};
