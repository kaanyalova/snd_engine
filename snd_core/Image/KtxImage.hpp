#pragma once

#include <ktx.h>

#include <cstdint>
#include <filesystem>
#include <span>

#include "snd_core/Image/Image.hpp"

class KtxImage {
  public:
    static auto from_file_path(const std::filesystem::path& file_path) -> Image;
    static auto from_bytes(std::span<const uint8_t> bytes) -> Image;
};
