#pragma once
#include <cinttypes>
#include <filesystem>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "Cli.hpp"
#include "CommandBuilder.hpp"

using CliArgType = std::variant<std::string, int64_t, std::filesystem::path>;

class CliBuilder {
  public:
    CliBuilder(int argc, char** argv);

    auto add_command(
        std::string_view name, std::optional<std::string_view> description = std::nullopt
    ) -> CommandBuilder;

    auto push_command(Command&& command) -> void;

    auto build() -> Cli;

  private:
    std::vector<Command> m_commands;
};
