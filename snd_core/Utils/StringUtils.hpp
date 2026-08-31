#pragma once
#include <string>

class StringUtils {
  public:
    static auto ltrim(std::string &string) -> void;
    static auto rtrim(std::string &string) -> void;
    static auto trim(std::string &string) -> void;

    static auto is_number(std::string_view string) -> bool;
};
