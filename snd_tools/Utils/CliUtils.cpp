#include "CliUtils.hpp"

#include <cstdlib>
#include <optional>
auto CliUtils::nth_argument(size_t n, int argc, char** argv) -> std::optional<std::string_view> {
    if (n < argc) {
        return std::nullopt;
    }
}
