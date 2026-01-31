#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Window.hpp"

struct RendererSettings {
    std::vector<const char*> required_extensions = {};
    std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    bool enable_validation = true;
    vk::Flags<vk::DebugUtilsMessageSeverityFlagBitsEXT> validation_log_level =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> validation_message_types =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    bool prefer_discrete_gpu = false;
};

struct FamilyIndices {
    uint32_t graphics_family;
    uint32_t presentation_family;
};

class Renderer {
  public:
    Renderer(Window& window, RendererSettings settings);
    auto run() -> void;

  private:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    Window& m_window;

    FamilyIndices m_family_indices;

    vk::SurfaceFormatKHR m_swap_chain_surface_format;
    vk::Extent2D m_swap_chain_extent;

    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debug_messenger = nullptr;
    vk::raii::PhysicalDevice m_physical_device = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::raii::Queue m_graphics_queue = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::Queue m_presentation_queue = nullptr;
    vk::raii::SwapchainKHR m_swap_chain = nullptr;
    std::vector<vk::Image> m_swap_chain_images = {};
    std::vector<vk::raii::ImageView> m_swap_chain_image_views = {};
    vk::raii::PipelineLayout m_pipeline_layout = nullptr;
    vk::raii::Pipeline m_graphics_pipeline = nullptr;

    std::vector<const char*> m_required_extensions;
    std::vector<const char*> m_validation_layers;
    bool m_enable_validation = true;
    vk::Flags<vk::DebugUtilsMessageSeverityFlagBitsEXT> m_validation_log_level;
    vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> m_validation_message_types;
    bool m_prefer_discrete_gpu = false;

    auto init_vulkan() -> void;

    auto create_instance() -> void;
    auto setup_debug_messenger() -> void;
    auto pick_physical_device() -> void;
    auto create_logical_device() -> void;
    auto create_surface() -> void;
    auto create_swap_chain() -> void;
    auto create_graphics_pipeline() -> void;

    auto are_required_extensions_supported_by_instance() -> bool;
    auto are_required_validation_layers_supported_by_instance() -> bool;
    auto get_validation_layers() -> std::vector<const char*>;
    auto is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool;
    auto find_queue_families(const vk::raii::PhysicalDevice& device) -> FamilyIndices;
    auto choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;

    auto create_image_views() -> void;

    [[nodiscard]] auto create_shader_module(const std::vector<char>& code) const
        -> vk::raii::ShaderModule;

    auto static choose_swap_surface_format(
        const std::vector<vk::SurfaceFormatKHR>& available_formats
    ) -> vk::SurfaceFormatKHR;
    auto static choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_modes)
        -> vk::PresentModeKHR;

    auto static debug_callback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
        void*
    ) -> vk::Bool32;

    auto main_loop() -> void;
    auto cleanup() -> void;
};
