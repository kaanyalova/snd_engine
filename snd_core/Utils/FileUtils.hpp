#pragma once

#include <cstdint>
#include <string>
#include <vector>

class FileUtils {
  public:
    static auto read_file(const std::string& file_path) -> std::vector<uint8_t>;
};