#include "Vertex.hpp"

#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"

auto Vertex::get_binding_description() -> vk::VertexInputBindingDescription {
    return vk::VertexInputBindingDescription {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };
}

auto Vertex::get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription> {
    return {
        vk::VertexInputAttributeDescription {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, position),
        },

        vk::VertexInputAttributeDescription {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, color),
        },

        vk::VertexInputAttributeDescription {
            .location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texture_coord),
        }

    };
}
