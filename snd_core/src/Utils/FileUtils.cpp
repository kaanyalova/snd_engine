#include "FileUtils.hpp"

#include <fstream>
#include <vector>

auto FileUtils::read_file(const std::string& file_path) -> std::vector<uint8_t> {
    auto file = std::ifstream(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::format("failed to open file: {}", file_path));
    }

    auto buffer = std::vector<uint8_t>(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    file.close();

    return buffer;
}