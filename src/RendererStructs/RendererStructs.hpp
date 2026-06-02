#pragma once
#include <glm/glm.hpp>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

enum class GpuPreference : uint8_t {
    Discrete,
    Integrated,
};

struct RendererSettings {
    std::vector<const char*> required_extensions = {};
    std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    bool enable_validation = true;
    vk::Flags<vk::DebugUtilsMessageSeverityFlagBitsEXT> validation_log_level =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

    vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> validation_message_types =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    //

    GpuPreference gpu_preference = GpuPreference::Discrete;
};

enum class BufferKind : uint8_t {
    Vertex,
    Index,
};

struct DeviceBufferInfo {
    BufferKind kind;
};

struct FamilyIndices {
    uint32_t graphics_family;
    uint32_t presentation_family;
};
