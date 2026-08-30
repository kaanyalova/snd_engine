#pragma once
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

class CliBuilder;

enum class CommandArgumentTypeIdentifier : uint8_t {
    String,
    Integer,
    Path,
};

using CommandArgumentType = std::variant<std::string, int64_t, std::filesystem::path>;

struct CommandArgument {
    std::string name;
    CommandArgumentTypeIdentifier type;
    std::optional<CommandArgumentType> default_value;
    bool is_positional;
    std::optional<std::string_view> description;
};

struct Command {
    std::string name;
    std::optional<std::string> short_name;
    std::vector<CommandArgument> arguments;
    std::optional<std::string> description;
    std::function<void(const Command& command)> on_execute;
};

class CommandBuilder {
  public:
    CommandBuilder(
        CliBuilder& builder,
        std::string_view name,
        std::optional<std::string_view> short_name,
        std::optional<std::string_view> description
    );
    auto add_arg(
        std::string_view name,
        CommandArgumentTypeIdentifier type,
        std::optional<CommandArgumentType> default_value = std::nullopt,
        bool is_positional = true,
        std::optional<std::string_view> description = std::nullopt
    ) -> CommandBuilder&;

    auto on_execute(std::function<void(const Command& command)> callback) -> void;

    auto build() -> CliBuilder&;

  private:
    CliBuilder& m_cli_builder;
    Command m_command;
};
