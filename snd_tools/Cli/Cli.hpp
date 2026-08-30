#pragma once
#include <optional>
#include <string_view>
#include <vector>

#include "CommandBuilder.hpp"

class Cli {
  public:
    explicit Cli(std::vector<Command> commands);
    auto run(int argc, char** argv) -> void;

  private:
    auto nth_arg(size_t n) -> std::optional<std::string_view>;
    auto args_into_string_views(int argc, char** argv) -> void;

    auto display_help_text() -> void;
    auto display_help_fragment_for_command(const Command& command) -> void;

    std::vector<Command> m_commands;
    std::vector<std::string_view> m_arguments;
};
