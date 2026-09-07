#pragma once
#include <span>

#include "Image.hpp"

class ImageUtils {
  public:
    static auto ensure_rgba8(Image&& image) -> Image;

  private:
    static auto convert_rgb8_to_rgba8(std::span<uint8_t> input) -> std::vector<uint8_t>;
};
