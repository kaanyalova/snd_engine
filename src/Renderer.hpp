#pragma once

#include <vector>
#include <vk_mem_alloc.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "RendererStructs.hpp"
#include "Vertex.hpp"
#include "Window.hpp"

class Renderer {
  public:
    Renderer(Window& window, RendererSettings settings);

    auto prepare() -> void;
    auto draw_frame() -> void;
    auto wait_idle() -> void;

    // auto allocate_gpu_memory(whatever) -> void;
    auto copy_buffer_waited(vk::raii::Buffer& from, vk::raii::Buffer& to, vk::DeviceSize size)
        -> void;

    [[nodiscard]] auto create_buffer(vk::DeviceSize size, BufferInfo info) const
        -> vma::raii::Buffer;

    [[nodiscard]] auto create_and_map_buffer(const void* data, vk::DeviceSize size, BufferInfo info)
        -> vma::raii::Buffer;

    [[nodiscard]] auto create_device_vertex_buffer(const void* data, vk::DeviceSize size)
        -> vma::raii::Buffer;

    [[nodiscard]] auto create_device_index_buffer(const void* data, vk::DeviceSize size)
        -> vma::raii::Buffer;

    ~Renderer();

  private:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

    uint32_t frame_index = 0;

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
    vk::raii::DescriptorSetLayout m_descriptor_set_layout = nullptr;
    vk::raii::Pipeline m_graphics_pipeline = nullptr;
    vk::raii::CommandPool m_command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_command_buffers = {};
    std::vector<vk::raii::Semaphore> m_present_complete_semaphores = {};
    std::vector<vk::raii::Semaphore> m_render_finished_semaphores = {};
    std::vector<vk::raii::Fence> in_flight_fences = {};

    // Memory allocator stuff
    vma::raii::Allocator m_allocator = nullptr;

    vma::raii::Buffer m_vertex_buffer = nullptr;
    vma::raii::Buffer m_index_buffer = nullptr;
    std::vector<vma::raii::Buffer> m_uniform_buffers = {};
    std::vector<void*> m_uniform_buffers_mapped = {};

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
    auto create_memory_allocator() -> void;
    auto create_surface() -> void;
    auto create_swap_chain() -> void;
    auto create_graphics_pipeline() -> void;
    auto create_command_pool() -> void;
    auto create_command_buffers() -> void;
    auto record_command_buffer(uint32_t image_index) -> void;
    auto create_sync_objects() -> void;
    auto create_vertex_buffer() -> void;
    auto create_index_buffer() -> void;
    auto create_descriptor_set_layout() -> void;
    auto create_uniform_buffers() -> void;

    auto transition_image_layout(
        uint32_t image_index,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 source_access_mask,
        vk::AccessFlags2 destination_access_mask,
        vk::PipelineStageFlags2 source_stage_mask,
        vk::PipelineStageFlags2 destination_stage_mask
    ) -> void;

    auto are_required_extensions_supported_by_instance() -> bool;
    auto are_required_validation_layers_supported_by_instance() -> bool;
    auto get_validation_layers() -> std::vector<const char*>;
    auto is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool;
    auto find_queue_families(const vk::raii::PhysicalDevice& device) -> FamilyIndices;
    auto choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;
    auto create_image_views() -> void;
    auto find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

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
