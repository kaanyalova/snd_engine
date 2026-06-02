#include "KtxImage.hpp"

#include <ktx.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string>
#include <vk_mem_alloc.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>

KtxImage::KtxImage(ktxTexture* texture, uint32_t width, uint32_t height, size_t size, uint8_t* data)
    : image(texture), size(size), width(width), height(height), data(data) {
}

auto KtxImage::from_file_name(const std::string& file_name) -> KtxImage {
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
    uint8_t* data = ktxTexture_GetData(texture);

    return KtxImage(texture, width, height, size, data);
}

KtxImage::~KtxImage() {
    ktxTexture_Destroy(image);
}
