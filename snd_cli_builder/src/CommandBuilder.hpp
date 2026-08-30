#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../snd_core/src/TemplateHelpers/IsLegitVariant.hpp"

class CliBuilder;

enum class CommandArgumentTypeIdentifier : uint8_t {
    String,
    Integer,
    Path,
};

using CommandArgumentType = std::variant<std::string, int64_t, std::filesystem::path>;

struct CommandArgument {
    std::string name;
    std::optional<std::string_view> short_name;
    CommandArgumentTypeIdentifier type;
    std::optional<CommandArgumentType> default_value;
    bool is_positional;
    std::optional<std::string_view> description;
};

struct OnExecuteArguments {
    std::vector<CommandArgumentType> arguments;
    std::unordered_map<std::string, size_t> named_argument_map;

    // these are helper functions to get the inner types of the CommandArgumentTypes
    // when writing the actual commands themselves
    template <typename T>
        requires LegitVariant<CommandArgumentType>
    auto get_named(const std::string& name) const -> std::optional<T> {
        auto it = named_argument_map.find(name);
        if (it == named_argument_map.end() || it->second >= arguments.size()) {
            return std::nullopt;
        }

        const auto& argument = &arguments[it->second];
        const T* unwrapped = std::get_if<T>(argument);
        if (unwrapped == nullptr) {
            return std::nullopt;
        }

        return std::make_optional(*unwrapped);
    }

    template <typename T>
        requires LegitVariant<CommandArgumentType>
    auto get_positional(size_t at) -> std::optional<T> {
        if (at >= arguments.size()) {
            return std::nullopt;
        }

        T* unwrapped = std::get_if<T>(&arguments[at]);
        if (unwrapped == nullptr) {
            return std::nullopt;
        }

        return std::make_optional(*unwrapped);
    }
};

struct Command {
    std::string name;
    std::vector<CommandArgument> arguments;
    std::optional<std::string> description;
    OnExecuteArguments on_execute_arguments;
    std::function<void(const OnExecuteArguments&)> on_execute;
};

class CommandBuilder {
  public:
    CommandBuilder(
        CliBuilder& builder, std::string_view name, std::optional<std::string_view> description
    );
    auto add_arg_with_default_value(
        std::string_view name,
        std::optional<std::string_view> short_name,
        CommandArgumentTypeIdentifier type,
        std::optional<CommandArgumentType> default_value = std::nullopt,
        std::optional<std::string_view> description = std::nullopt,
        bool is_positional = true
    ) -> CommandBuilder&;

    auto on_execute(std::function<void(const OnExecuteArguments& execute_arguments)> callback)
        -> CommandBuilder&;

    auto build() -> CliBuilder&;

  private:
    CliBuilder& m_cli_builder;
    Command m_command;
};
