#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "snd_core/Resources/Scene/SceneData.hpp"

// Gpu struct, mirrors PrimitiveInstance in shaders/DataTypes.slang
struct BakedPrimitiveInstance {
    uint32_t primitive_index;
    uint32_t node_transform_index;
    glm::mat4x4 transform;
};

struct BakedScene {
    std::vector<IndexSpan> primitive_draw_ranges;
    std::vector<BakedPrimitiveInstance> primitive_instances;
    std::vector<glm::mat4x4> scene_node_transforms;

    auto get_stats() -> std::string;
};