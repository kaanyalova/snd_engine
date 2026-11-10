
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <print>
#include <string>

#include "snd_cli_builder/CliBuilder.hpp"
#include "snd_core/Renderer/Vulkan/Resources/VulkanScene.hpp"
#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"
#include "snd_core/Resources/GltfLoader.hpp"
#include "snd_core/Resources/Scene/Scene.hpp"

auto main(int argc, char** argv) -> int {
    auto builder = CliBuilder(argc, argv);

    auto cli =
        builder.add_command("dump-gltf", "Dump a .gltf file into a custom scene format")
            .add_arg_with_default_value("input", "i", CommandArgumentTypeIdentifier::Path, std::nullopt, "Input file")
            .add_arg_with_default_value("output", "o", CommandArgumentTypeIdentifier::Path, std::nullopt, "Output file")
            .on_execute([](const OnExecuteArguments& args) {
                GltfLoader loader = GltfLoader("./");
                SceneData scene_data;
                auto path = args.get_named<std::filesystem::path>("input").value();
                loader.load_into_scene_from_path(scene_data, path);

                auto scene = Scene();
                scene.push_scene_data(scene_data);
                auto range = scene.get_models()[0];

                scene.get_root_node().add_model_component(ModelComponent(range));

                SceneNode node = SceneNode();
                glm::mat4 transform = glm::mat4(1.0f);
                transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 100.0f));
                node.set_transform(transform);
                node.add_model_component(ModelComponent(range));

                scene.get_root_node().add_child(std::move(node));
                scene.bake_scene();

                auto window = Window("test", 640, 480);
                auto renderer = VulkanRenderer(window, RendererSettings {.enable_validation = true});
                auto gpu_scene = VulkanScene(renderer, scene);

                std::println("{}", scene.get_stats());
                renderer.submit_single_command(
                    [&](vk::raii::CommandBuffer& command_buffer) { gpu_scene.run_create_commands(command_buffer); },
                    nullptr
                );
                renderer.get_device().inner().waitIdle();
            })
            .build()
            .build();

    cli.run(argc, argv);
}
