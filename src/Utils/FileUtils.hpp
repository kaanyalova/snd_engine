#pragma once

#include <string_view>
#include <vector>

class FileUtils {
  public:
    static auto read_file(const std::string& file_path) -> std::vector<char>;
};