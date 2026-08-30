add_executable(snd_tools
        main.cpp
        Utils/CliUtils.cpp
        Utils/CliUtils.hpp
        Cli/CliBuilder.hpp
        Cli/CliBuilder.cpp
        Cli/CommandBuilder.cpp
        Cli/CommandBuilder.hpp
        Cli/Cli.cpp
        Cli/Cli.hpp
)

target_link_libraries(snd_tools PRIVATE snd_core)
