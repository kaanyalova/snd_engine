#pragma once
#include <optional>
#include <string_view>

class CliUtils {
    static auto nth_argument(size_t n, int argc, char** argv) -> std::optional<std::string_view>;
};
