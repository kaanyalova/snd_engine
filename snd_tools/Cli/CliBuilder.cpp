
#include "CliBuilder.hpp"

CliBuilder::CliBuilder(int argc, char** argv) {
}

auto CliBuilder::add_command(
    std::string_view name,
    std::optional<std::string_view> short_name,
    std::optional<std::string_view> description
) -> CommandBuilder {
    return CommandBuilder(*this, name, short_name, description);
}

auto CliBuilder::push_command(Command&& command) -> void {
    m_commands.emplace_back(command);
}
auto CliBuilder::build() -> Cli {
    return Cli(std::move(m_commands));
}