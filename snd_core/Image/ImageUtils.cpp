#include "ImageUtils.hpp"

#include <stdexcept>
auto ImageUtils::ensure_rgba8(Image&& image) -> Image {
    switch (image.format) {
        case ImageFormat::Undefined:
            throw std::runtime_error("undefined format");
        case ImageFormat::Rgba8:
            return std::move(image);
        case ImageFormat::Rgb8:
            break;
    }
}
auto ImageUtils::convert_rgb8_to_rgba8(std::span<uint8_t> input) -> std::vector<uint8_t> {
}