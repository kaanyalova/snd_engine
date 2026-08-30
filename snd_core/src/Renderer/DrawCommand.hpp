#pragma once
#include <cstdint>
#include <glm/glm.hpp>

#include "../Resources/SceneData.hpp"

struct DrawCommand {
    uint32_t material_id;
    uint32_t vertex_count;
    glm::mat4 transform;
    Vertex* vertices;
};