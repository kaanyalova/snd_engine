#include "CommandBuilder.hpp"

#include "CliBuilder.hpp"

CommandBuilder::CommandBuilder(
    CliBuilder& builder, std::string_view name, std::optional<std::string_view> description
)
    : m_cli_builder(builder) {
    m_command.name = std::string(name);
    m_command.description = std::optional(description);
}

auto CommandBuilder::add_arg_with_default_value(
    std::string_view name,
    std::optional<std::string_view> short_name,
    CommandArgumentTypeIdentifier type,
    std::optional<CommandArgumentType> default_value,
    std::optional<std::string_view> description,
    bool is_positional
) -> CommandBuilder& {
    auto command_argument = CommandArgument {
        .name = std::string(name),
        .short_name = std::move(short_name),
        .type = type,
        .default_value = std::move(default_value),
        .is_positional = is_positional,
        .description = std::optional(description)
    };

    m_command.arguments.emplace_back(std::move(command_argument));

    return *this;
}
auto CommandBuilder::on_execute(
    std::function<void(const OnExecuteArguments& execute_arguments)> callback
) -> CommandBuilder& {
    m_command.on_execute = std::move(callback);
    return *this;
}

auto CommandBuilder::build() -> CliBuilder& {
    m_cli_builder.push_command(std::move(m_command));
    return m_cli_builder;
}