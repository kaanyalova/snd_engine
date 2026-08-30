#include "CommandBuilder.hpp"

#include "CliBuilder.hpp"

CommandBuilder::CommandBuilder(
    CliBuilder& builder,
    std::string_view name,
    std::optional<std::string_view> short_name,
    std::optional<std::string_view> description
)
    : m_cli_builder(builder) {
    m_command.name = std::string(name);
    m_command.short_name = std::optional(short_name);
    m_command.description = std::optional(description);
}

auto CommandBuilder::add_arg(
    std::string_view name,
    CommandArgumentTypeIdentifier type,
    std::optional<CommandArgumentType> default_value,
    bool is_positional,
    std::optional<std::string_view> description
) -> CommandBuilder& {
    auto command_argument = CommandArgument {
        .name = std::string(name),
        .type = type,
        .default_value = std::move(default_value),
        .is_positional = is_positional,
        .description = std::optional(description)
    };

    m_command.arguments.emplace_back(std::move(command_argument));

    return *this;
}
auto CommandBuilder::on_execute(std::function<void(const Command& command)> callback) -> void {
    m_command.on_execute = std::move(callback);
}

auto CommandBuilder::build() -> CliBuilder& {
    m_cli_builder.push_command(std::move(m_command));
    return m_cli_builder;
}