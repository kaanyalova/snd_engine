#include "BakedScene.hpp"

#include <format>

auto BakedScene::get_stats() -> std::string {
    std::string result;

    result += "----------Baked Scene Stats----------\n";
    for (size_t i = 0; i < primitive_draw_ranges.size(); i++) {
        const auto& range = primitive_draw_ranges[i];

        result += std::format(
            "Primitive range {} [{}..{}] ({} instances) \n", i, range.index, range.index + range.length, range.length
        );
    }

    result += "--------------------\n";

    result += std::format("{} node transforms \n", scene_node_transforms.size());

    return result;
}