#include "Cli.hpp"

#include <print>
#include <utility>
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
    CommandBuilder()
}
auto Cli::display_help_text() -> void {
}
auto Cli::display_help_fragment_for_command(const Command& command) -> void {
    std::string short_name_formatted =
        command.short_name
            .and_then([](std::string_view s) { return std::optional(std::format(", {}", s)); })
            .value_or("");

    std::println("{}{}", command.name, short_name_formatted);
}