#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static auto get_binding_description() -> vk::VertexInputBindingDescription;
    static auto get_attribute_descriptions() -> std::array<vk::VertexInputAttributeDescription, 2>;
};