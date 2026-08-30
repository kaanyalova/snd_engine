add_executable(snd_tools
        src/main.cpp
)

target_link_libraries(snd_tools PRIVATE snd_cli_builder snd_core)
