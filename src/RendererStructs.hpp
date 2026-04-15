#pragma once
#include <glm/glm.hpp>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"

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

    bool prefer_discrete_gpu = false;
};

struct BufferInfo {
    vk::BufferUsageFlags buffer_usage;
    vma::MemoryUsage allocation_usage;
    vma::AllocationCreateFlags allocation_flags;

    static auto vertex_staging_buffer() -> BufferInfo;
    static auto vertex_device_buffer() -> BufferInfo;

    static auto index_staging_buffer() -> BufferInfo;
    static auto index_device_buffer() -> BufferInfo;

    static auto default_staging_buffer() -> BufferInfo;

    static auto uniform_buffer() -> BufferInfo;

    static auto image_staging_buffer() -> BufferInfo;
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

struct UniformBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct ImageInfo {
    vk::Format format;
    vk::ImageTiling tiling;
    vk::ImageUsageFlags usage_flags;
    vk::MemoryPropertyFlags memory_flags;

    static auto r8g8b8a8Srgb() -> ImageInfo;
};

struct ImageTransitionInfo {
    vk::ImageLayout from;
    vk::ImageLayout to;

    static auto undefined_to_dst_optimal() -> ImageTransitionInfo;
    static auto dst_optimal_to_shader_optimal() -> ImageTransitionInfo;
};
