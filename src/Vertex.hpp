#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 texture_coord;

    static auto get_binding_description() -> vk::VertexInputBindingDescription;
    static auto get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription>;
};