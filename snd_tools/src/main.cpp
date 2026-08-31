
#include <iostream>
#include <print>
#include <string>

#include "snd_core/Resources/GltfLoader.hpp"
#include "snd_cli_builder/CliBuilder.hpp"

auto main(int argc, char** argv) -> int {
    auto builder = CliBuilder(argc, argv);

    auto cli =
        builder.add_command("dump-gltf", "Dump a .gltf file into a custom scene format")
            .add_arg_with_default_value(
                "input", "i", CommandArgumentTypeIdentifier::Path, std::nullopt, "Input file"
            )
            .add_arg_with_default_value(
                "output", "o", CommandArgumentTypeIdentifier::Path, std::nullopt, "Output file"
            )
            .on_execute([](const OnExecuteArguments& args) {
                GltfLoader loader = GltfLoader("./");
                SceneData scene_data;
                auto path = args.get_named<std::filesystem::path>("input").value();
                loader.load_into_scene_from_path(scene_data, path);
                std::string stats = scene_data.get_scene_stats();
                std::println("{}", stats);
            })
            .build()
            .build();

    cli.run(argc, argv);
}
