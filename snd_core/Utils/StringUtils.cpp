#include "snd_core/Utils/StringUtils.hpp"

#include <algorithm>
#include <ranges>

auto StringUtils::ltrim(std::string &string) -> void {
    string.erase(string.begin(), std::ranges::find_if(string, [](unsigned char char_) {
                     return !std::isspace(char_);
                 }));
}

auto StringUtils::rtrim(std::string &string) -> void {
    string.erase(
        std::ranges::find_if(
            std::ranges::reverse_view(string),
            [](unsigned char char_) { return !std::isspace(char_); }
        ).base(),
        string.end()
    );
}

auto StringUtils::trim(std::string &string) -> void {
    ltrim(string);
    rtrim(string);
}

auto StringUtils::is_number(std::string_view string) -> bool {
    return !string.empty() && std::ranges::find_if(string, [](unsigned char c) {
                                  return !std::isdigit(c);
                              }) == string.end();
}
