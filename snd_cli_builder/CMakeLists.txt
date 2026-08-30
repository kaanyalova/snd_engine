add_library(snd_cli_builder STATIC
        src/Cli.cpp
        src/Cli.hpp
        src/CommandBuilder.cpp
        src/CommandBuilder.hpp
        src/CliBuilder.cpp
        src/CliBuilder.hpp
)

target_include_directories(snd_cli_builder PUBLIC src)

target_link_libraries(snd_cli_builder PRIVATE snd_core)
