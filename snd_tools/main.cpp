
#include "Cli/CliBuilder.hpp"
auto main(int argc, char** argv) -> int {
    auto builder = CliBuilder(argc, argv);

    auto cli = builder.add_command("some_command", "s")
                   .add_arg("some_arg", CommandArgumentTypeIdentifier::String)
                   .add_arg("other_arg", CommandArgumentTypeIdentifier::Integer)
                   .build()
                   .build();

    cli.run(argc, argv);
}
