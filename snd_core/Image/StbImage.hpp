#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

#include "snd_core/Image/Image.hpp"

class StbImage {
  public:
    static auto from_file_path(const std::filesystem::path& file_path) -> Image;
    static auto from_bytes(std::span<const uint8_t> bytes) -> Image;

  private:
    static auto convert_stbi_composition_to_image_format(int composition) -> ImageFormat;
};