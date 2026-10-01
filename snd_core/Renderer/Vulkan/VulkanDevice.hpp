#pragma once

#include <cstdint>
#include <vector>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "snd_core/Window.hpp"

enum class GpuPreference : uint8_t {
    Discrete,
    Integrated,
};

struct FamilyIndices {
    uint32_t graphics;
    uint32_t presentation;
};

struct DeviceQueues {
    vk::raii::Queue graphics = nullptr;
    vk::raii::Queue presentation = nullptr;
};

struct DeviceCreationInfo {
    bool enable_validation = false;
    GpuPreference gpu_preference = GpuPreference::Integrated;
    std::vector<const char*> sdl_required_extensions = {};
};

class VulkanDevice {
  public:
    VulkanDevice() = delete;

    VulkanDevice(const DeviceCreationInfo& info, Window& window);

    auto inner() -> vk::raii::Device& { return m_device; }

    auto get_device_address(const vk::Buffer& buffer) -> vk::DeviceAddress;
    auto get_context() -> vk::raii::Context& { return m_vulkan_context; }
    auto get_instance() -> vk::raii::Instance& { return m_instance; }
    auto get_physical_device() -> vk::raii::PhysicalDevice& { return m_physical_device; }
    auto get_allocator() -> vma::raii::Allocator& { return m_allocator; }
    auto get_command_pool() -> vk::raii::CommandPool& { return m_command_pool; }
    auto get_surface() -> vk::raii::SurfaceKHR& { return m_surface; }
    auto get_depth_format() -> vk::Format { return m_depth_format; }
    auto get_family_indices() -> FamilyIndices& { return m_family_indices; }
    auto get_queues() -> DeviceQueues& { return m_queues; }

  private:
    const DeviceCreationInfo& m_info;

    vk::raii::Context m_vulkan_context = {};
    vk::raii::Instance m_instance = nullptr;
    vk::raii::PhysicalDevice m_physical_device = nullptr;
    vk::raii::Device m_device = nullptr;
    vma::raii::Allocator m_allocator = nullptr;
    vk::raii::CommandPool m_command_pool = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::Format m_depth_format = vk::Format::eUndefined;
    FamilyIndices m_family_indices = {};
    DeviceQueues m_queues = {};
    vk::raii::DebugUtilsMessengerEXT m_debug_messenger = nullptr;

    vk::Flags<vk::DebugUtilsMessageSeverityFlagBitsEXT> m_validation_log_level =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

    vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> m_validation_message_types =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    Window& m_window;

    std::vector<const char*> m_required_extensions;
    std::vector<const char*> m_validation_layers = {};
    float m_max_sampler_anisotropy = 1;

    auto create_instance() -> void;
    auto create_physical_device() -> void;
    auto get_physical_device_properties() -> void;
    auto find_family_indices() -> void;
    auto create_logical_device() -> void;
    auto create_memory_allocator() -> void;
    auto find_depth_format() -> void;
    auto create_surface() -> void;
    auto setup_debug_messenger() -> void;
    auto create_command_pool() -> void;

    static auto debug_callback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
        void* _user_data
    ) -> vk::Bool32;

    auto are_required_extensions_supported_by_instance() -> bool;
    auto is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool;
};
