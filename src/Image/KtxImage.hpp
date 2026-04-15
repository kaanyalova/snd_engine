#pragma once

#include <ktx.h>

#include <cstdint>
#include <string>

class KtxImage {
  public:
    static auto from_file_name(const std::string& file_name) -> KtxImage;

    ktxTexture* image;

    size_t size;
    uint32_t width;
    uint32_t height;
    uint8_t* data;

    ~KtxImage();

  private:
    KtxImage(uint32_t width, uint32_t height, size_t size, uint8_t* data);
};
