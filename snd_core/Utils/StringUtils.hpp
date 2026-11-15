#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

class StringUtils {
  public:
    static auto ltrim(std::string &string) -> void;
    static auto rtrim(std::string &string) -> void;
    static auto trim(std::string &string) -> void;

    static auto is_number(std::string_view string) -> bool;

    template <typename... Bases>
    struct overload : Bases... {
        using is_transparent = void;
        using Bases::operator()...;
    };

    struct CharPointerHash {
        auto operator()(const char *ptr) const noexcept { return std::hash<std::string_view> {}(ptr); }
    };

    using TransparentStringHash = overload<std::hash<std::string>, std::hash<std::string_view>, CharPointerHash>;
};
