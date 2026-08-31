#pragma once
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

#include "snd_core/Utils/StringUtils.hpp"
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

    /**
     * parse the command line arguments after the initial into typed variants
     * based on the CommandArgument definitions of the given command
     *
     * this also checks if the names for these arguments do exist in the command as well
     *
     *
     * @param command currently selected command
     * @param cli_arguments the raw arguments array
     * @return
     *  if successful:
     *      checked variants of the arguments, along with named values
     *  else:
     *      an error message
     */
    auto process_arguments(const Command& command, std::vector<std::string_view> cli_arguments)
        -> std::expected<OnExecuteArguments, std::string>;

    /**
     * Convert a string_view into one of the valid CommandArgumentTypes
     * @param argument_value the argument
     * @param type the type to be converted into
     * @return converted value
     */
    auto try_convert_argument_value_to_type(
        std::string_view argument_value, CommandArgumentTypeIdentifier type
    ) -> std::optional<CommandArgumentType> {
        switch (type) {
            case CommandArgumentTypeIdentifier::String: {
                return std::make_optional(std::string(argument_value));
            }

            case CommandArgumentTypeIdentifier::Integer: {
                bool is_number = StringUtils::is_number(argument_value);
                if (!is_number) {
                    return std::nullopt;
                }

                // stoi doesn't work on string_views
                return std::make_optional(std::stoi(std::string(argument_value)));
            }

            case CommandArgumentTypeIdentifier::Path: {
                auto path = std::filesystem::path(argument_value);
                return std::make_optional(path);
            }
        }

        return std::nullopt;
    }

    auto does_argument_with_name_exist(std::string_view name, const Command& command) -> bool;
    auto does_argument_with_short_name_exist(std::string_view name, const Command& command) -> bool;

    static auto get_command_type_identifier_name(CommandArgumentTypeIdentifier identifier)
        -> std::string_view;

    std::vector<Command> m_commands;
    std::vector<std::string_view> m_arguments;
};
