#include "FileUtils.hpp"

#include <fstream>
#include <vector>

auto FileUtils::read_file(const std::string& file_path) -> std::vector<char> {
    auto file = std::ifstream(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + file_path);
    }

    auto buffer = std::vector<char>(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), buffer.size());
    file.close();

    return buffer;
}