#include "Cli.hpp"

#include <algorithm>
#include <expected>
#include <iostream>
#include <print>
#include <string_view>
#include <utility>

#include "snd_core/Utils/StringUtils.hpp"

using namespace std::literals;

auto Cli::nth_arg(size_t n) -> std::optional<std::string_view> {
    if (m_arguments.size() < n) {
        return std::nullopt;
    }

    return m_arguments[n];
}

auto Cli::args_into_string_views(int argc, char** argv) -> void {
    for (size_t i = 0; i < argc; i++) {
        m_arguments.push_back(std::string_view(argv[i]));
    }
}

Cli::Cli(std::vector<Command> commands) : m_commands(std::move(commands)) {
}

auto Cli::run(int argc, char** argv) -> void {
    args_into_string_views(argc, argv);

    if (m_arguments.size() < 2) {
        display_help_text();
        exit(0);
    }

    std::string_view command_name = m_arguments[1];

    auto current_command =
        std::ranges::find_if(m_commands, [&](const Command& c) { return c.name == command_name; });

    if (command_name == "help"sv) {
        display_help_text();
        exit(0);
    }

    if (current_command == m_commands.end()) {
        std::println("Command not found, run help to get the list of commands");
        exit(0);
    }

    auto processed_arguments = process_arguments(*current_command, m_arguments);
    if (!processed_arguments.has_value()) {
        std::println("{}", processed_arguments.error());
    }

    current_command->on_execute_arguments = processed_arguments.value();
    current_command->on_execute(current_command->on_execute_arguments);
}

auto Cli::display_help_text() -> void {
    std::println("Commands:");
    for (auto& command : m_commands) {
        display_help_fragment_for_command(command);
    }
}
auto Cli::display_help_fragment_for_command(const Command& command) -> void {
    std::print("{}:", command.name);

    for (auto& argument : command.arguments) {
        std::string short_name_formatted =
            argument.short_name
                .and_then([](std::string_view s) { return std::optional(std::format(", -{}", s)); })
                .value_or("");
        std::string names_formatted = std::format("--{}{}", argument.name, short_name_formatted);

        std::print(" {} ", names_formatted);
        std::print("<{}>", get_command_type_identifier_name(argument.type));
    }

    /*
    std::string short_name_formatted =
        command.short_name
            .and_then([](std::string_view s) { return std::optional(std::format(", {}", s)); })
            .value_or("");
    std::string names_formatted = std::format("{}{}", command.name, short_name_formatted);

    std::println("{:<20}  {:>40}", names_formatted, command.description.value_or(""));
    */
}
auto Cli::process_arguments(const Command& command, std::vector<std::string_view> cli_arguments)
    -> std::expected<OnExecuteArguments, std::string> {
    OnExecuteArguments on_execute_arguments;

    const std::vector<CommandArgument>& command_arguments = command.arguments;

    // this is the index of the raw command line parameters
    // it start at 2 assuming index 0 is the path to the binary and index 1 is
    // the command
    size_t raw_argument_index = 2;
    // this is the index of the current parameter and its value in the Command
    size_t argument_index = 0;

    while (raw_argument_index < cli_arguments.size() && argument_index < command_arguments.size()) {
        std::string_view current_raw_argument = cli_arguments[raw_argument_index];
        const CommandArgument& current_argument = command_arguments[argument_index];

        // a long argument that starts with "--"
        if (current_raw_argument.starts_with("--")) {
            std::string_view argument_name = current_raw_argument;
            argument_name.remove_prefix(2);

            if (!does_argument_with_name_exist(argument_name, command)) {
                return std::unexpected(
                    std::format("argument with name {} does not exist", argument_name)
                );
            }

            std::string_view argument_value = cli_arguments[raw_argument_index + 1];

            // try to convert the value to the expected type
            CommandArgumentTypeIdentifier expected_type = current_argument.type;
            auto converted_maybe =
                try_convert_argument_value_to_type(argument_value, expected_type);

            if (!converted_maybe.has_value()) {
                return std::unexpected(
                    std::format(
                        "the long argument at index {} was excepted to have the type of {}",
                        argument_value,
                        get_command_type_identifier_name(expected_type)
                    )
                );
            }

            // add to the argument list
            on_execute_arguments.named_argument_map[std::string(argument_name)] =
                on_execute_arguments.arguments.size();
            on_execute_arguments.arguments.emplace_back(converted_maybe.value());

            raw_argument_index += 2;
            argument_index += 1;
            continue;
        }

        // a short argument that start with "-"
        if (current_raw_argument.starts_with("-")) {
            std::string_view argument_name = current_raw_argument;
            argument_name.remove_prefix(1);

            if (!does_argument_with_short_name_exist(argument_name, command)) {
                return std::unexpected(
                    std::format("argument with short name {} does not exist", argument_name)
                );
            }

            std::string_view argument_value = cli_arguments[raw_argument_index + 1];

            // try to convert the value to the expected type
            CommandArgumentTypeIdentifier expected_type = current_argument.type;
            auto converted_maybe =
                try_convert_argument_value_to_type(argument_value, expected_type);

            if (!converted_maybe.has_value()) {
                return std::unexpected(
                    std::format(
                        "the short argument at index {} was excepted to have the type of {}",
                        argument_value,
                        get_command_type_identifier_name(expected_type)
                    )
                );
            }

            // now we need the long name of the argument to add it to the named arguments
            auto found_argument =
                std::ranges::find_if(command.arguments, [&](const CommandArgument& arg) {
                    return arg.short_name.value_or("") == argument_name;
                });

            if (found_argument == command.arguments.end()) {
                return std::unexpected(
                    std::format("the short argument at index {} was not expected", argument_name)
                );
            }

            // add to the argument list
            on_execute_arguments.named_argument_map[std::string(found_argument->name)] =
                on_execute_arguments.arguments.size();
            on_execute_arguments.arguments.emplace_back(converted_maybe.value());

            raw_argument_index += 2;
            argument_index += 1;
            continue;
        }

        // then we look for the positional arguments
        if (current_argument.is_positional) {
            std::string_view argument_value = cli_arguments[raw_argument_index];

            // try to convert the value to the expected type
            CommandArgumentTypeIdentifier expected_type = current_argument.type;
            auto converted_maybe =
                try_convert_argument_value_to_type(argument_value, expected_type);

            if (!converted_maybe.has_value()) {
                return std::unexpected(
                    std::format(
                        "the positional argument at index {} was excepted to have the type of {}",
                        argument_value,
                        get_command_type_identifier_name(expected_type)
                    )
                );
            }

            // add to the argument list
            on_execute_arguments.named_argument_map[std::string(current_argument.name)] =
                on_execute_arguments.arguments.size();
            on_execute_arguments.arguments.emplace_back(converted_maybe.value());

            raw_argument_index += 1;
            argument_index += 1;
        }
    }

    return on_execute_arguments;
}

auto Cli::does_argument_with_name_exist(std::string_view name, const Command& command) -> bool {
    return std::ranges::find_if(command.arguments, [&](const CommandArgument& arg) {
               return arg.name == name;
           }) != command.arguments.end();
}

auto Cli::does_argument_with_short_name_exist(std::string_view name, const Command& command)
    -> bool {
    return std::ranges::find_if(command.arguments, [&](const CommandArgument& arg) {
               return arg.short_name.value_or("") == name;
           }) != command.arguments.end();
}

auto Cli::get_command_type_identifier_name(CommandArgumentTypeIdentifier identifier)
    -> std::string_view {
    switch (identifier) {
        case CommandArgumentTypeIdentifier::String:
            return "String";
        case CommandArgumentTypeIdentifier::Integer:
            return "Integer";
        case CommandArgumentTypeIdentifier::Path:
            return "Path";
    }
    return "What";
}