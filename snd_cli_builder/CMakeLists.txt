add_library(snd_cli_builder STATIC
        Cli.cpp
        Cli.hpp
        CommandBuilder.cpp
        CommandBuilder.hpp
        CliBuilder.cpp
        CliBuilder.hpp
)

# include dir is the parent so includes read "snd_cli_builder/CliBuilder.hpp"
target_include_directories(snd_cli_builder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(snd_cli_builder PRIVATE snd_core)
