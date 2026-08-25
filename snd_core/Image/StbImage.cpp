#include "StbImage.hpp"

#include <cstdint>
#include <print>
#include <span>
#include <vector>

#include "../Utils/FileUtils.hpp"
#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

auto StbImage::from_file_path(const std::filesystem::path& file_path) -> Image {
    std::vector<uint8_t> file_data = FileUtils::read_file(file_path);
    return StbImage::from_bytes(std::span<const uint8_t>(file_data));
}

auto StbImage::from_bytes(std::span<const uint8_t> bytes) -> Image {
    int x;
    int y;
    int composition;

    uint8_t* data = stbi_load_from_memory(
        bytes.data(), static_cast<int>(bytes.size()), &x, &y, &composition, STBI_default
    );

    if (data == nullptr) {
        throw std::runtime_error("failed to load image from bytes");
    }

    auto data_size = static_cast<size_t>(x * y * composition);

    ImageFormat format = convert_stbi_composition_to_image_format(composition);

    auto image = Image {
        .width = static_cast<uint32_t>(x),
        .height = static_cast<uint32_t>(y),
        .data = std::vector<uint8_t>(data, data + data_size),
        .format = format,
    };

    stbi_image_free(data);
    return image;
}

auto StbImage::convert_stbi_composition_to_image_format(int composition) -> ImageFormat {
    switch (composition) {
        case STBI_rgb_alpha:
            return ImageFormat::Rgba8;
        case STBI_rgb:
            return ImageFormat::Rgb8;
        default:
            std::println("The (stb) image has {} components, which is unsupported", composition);
            return ImageFormat::Undefined;
    }
}
