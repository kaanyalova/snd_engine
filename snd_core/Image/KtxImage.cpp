#include "KtxImage.hpp"

#include <ktx.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

auto KtxImage::from_file_path(const std::filesystem::path& file_name) -> Image {
    ktxTexture* texture;

    KTX_error_code result = ktxTexture_CreateFromNamedFile(
        file_name.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture
    );

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("failed to load texture");
    }

    uint32_t width = texture->baseWidth;
    uint32_t height = texture->baseHeight;
    size_t size = ktxTexture_GetImageSize(texture, 0);
    uint8_t* data_pointer = ktxTexture_GetData(texture);
    auto data = std::vector<uint8_t>(data_pointer, data_pointer + size);

    return Image {
        .width = width,
        .height = height,
        .data = std::move(data),
    };
}

auto KtxImage::from_bytes(std::span<const uint8_t> bytes) -> Image {
    ktxTexture* texture;

    KTX_error_code result = ktxTexture_CreateFromMemory(
        bytes.data(), bytes.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture
    );

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("failed to load texture");
    }

    uint32_t width = texture->baseWidth;
    uint32_t height = texture->baseHeight;
    size_t size = ktxTexture_GetImageSize(texture, 0);
    uint8_t* data_pointer = ktxTexture_GetData(texture);
    auto data = std::vector<uint8_t>(data_pointer, data_pointer + size);

    ktxTexture_Destroy(texture);

    return Image {
        .width = width,
        .height = height,
        .data = std::move(data),
    };
}