#include "snd_core/Resources/Scene/SceneData.hpp"

#include <format>
#include <string>

auto ImageData::view_of(std::span<const uint8_t> data) const -> ImageView {
    return ImageView {
        .width = width,
        .height = height,
        .data = std::span(&data[data_span.index], data_span.length),
        .format = format,
    };
}
auto SceneData::get_scene_stats() -> std::string {
    std::string scene_stats = std::format(
        "Scene stats:\n {}  primitive instances\n {} primitives\n {} materials\n {} "
        "textures\n {} samplers\n {} vertices\n {} indices\n {:.2f}mb image data\n {:.2f}mb vertex "
        "data",
        primitive_instances.size(),
        primitives.size(),
        materials.size(),
        textures.size(),
        samplers.size(),
        vertices.size(),
        indices.size(),
        static_cast<double>(image_data.size()) / 1024.0 / 1024.0,
        static_cast<double>(vertices.size() * sizeof(Vertex)) / 1024.0 / 1024.0
    );

    return scene_stats;
}