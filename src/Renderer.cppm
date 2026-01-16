export module Renderer;

import std;
import std.compat;
import vulkan;

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

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(const vk::raii::Instance& instance)>;

class Renderer {
  public:
    Renderer(CreateSurfaceFunc create_surface, RendererSettings settings);
    auto run() -> void;

  private:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    vk::raii::Context m_context;
    vk::raii::Instance m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debug_messenger = nullptr;
    vk::raii::PhysicalDevice m_physical_device = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::raii::Queue m_graphics_queue = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;

    CreateSurfaceFunc m_create_surface_func = nullptr;

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

    auto are_required_extensions_supported_by_instance() -> bool;
    auto are_required_validation_layers_supported_by_instance() -> bool;
    auto get_validation_layers() -> std::vector<const char*>;
    auto is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool;
    auto find_queue_families(const vk::raii::PhysicalDevice& device) -> uint32_t;

    auto static debug_callback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* callback_data, void this_*)
        -> vk::Bool32;

    auto main_loop() -> void;
    auto cleanup() -> void;
};