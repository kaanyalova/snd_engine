#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <vk_mem_alloc.hpp>
#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "../Vertex.hpp"
#include "../Window.hpp"
#include "./RendererStructs/BufferInfo.hpp"
#include "./RendererStructs/ImageInfo.hpp"
#include "./RendererStructs/ImageTransitionInfo.hpp"
#include "./RendererStructs/ImageViewInfo.hpp"
#include "./RendererStructs/RendererStructs.hpp"
#include "Device/Device.hpp"
#include "Swapchain/Swapchain.hpp"

class VulkanRenderer {
  public:
    VulkanRenderer(Window& window, RendererSettings settings);

    auto prepare() -> void;
    auto process() -> void;
    auto wait_idle() -> void;
    auto recreate_swap_chain() -> void;

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

    [[nodiscard]] auto create_image(uint32_t width, uint32_t height, ImageInfo info)
        -> vma::raii::Image;

    [[nodiscard]] auto create_and_map_image(
        void* data, uint32_t width, uint32_t height, ImageInfo info
    ) -> vma::raii::Image;

    auto run_single_time_commands(
        std::function<void(vk::raii::CommandBuffer& command_buffer)> commands
    ) -> void;

    auto create_image_view(const vk::raii::Image& image, ImageViewInfo info) -> vk::raii::ImageView;

    auto create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo;

    ~VulkanRenderer();

  private:
    auto draw_frame() -> void;

    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<Vertex> VERTICES = {
        {.position = {-0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}, .texture_coord = {0.0f, 0.0f}},
        {.position = {0.5f, -0.5f}, .color = {0.0f, 1.0f, 0.0f}, .texture_coord = {1.0f, 0.0f}},
        {.position = {0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f}, .texture_coord = {1.0f, 1.0f}},
        {.position = {-0.5f, 0.5f}, .color = {1.0f, 1.0f, 1.0f}, .texture_coord = {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> INDICES = {0, 1, 2, 2, 3, 0};

    const std::string TEXTURE_PATH = "";

    uint32_t frame_index = 0;

    Window& m_window;

    Swapchain m_swapchain;
    VulkanDevice m_device;

    FamilyIndices m_family_indices;
    vk::raii::Queue m_graphics_queue = nullptr;
    vk::raii::Queue m_presentation_queue = nullptr;

    vk::SurfaceFormatKHR m_swap_chain_surface_format;
    vk::Extent2D m_swap_chain_extent;
    vk::raii::SurfaceKHR m_surface = nullptr;

    vk::raii::DebugUtilsMessengerEXT m_debug_messenger = nullptr;

    vk::raii::SwapchainKHR m_swap_chain = nullptr;
    std::vector<vk::Image> m_swap_chain_images = {};
    std::vector<vk::raii::ImageView> m_swap_chain_image_views = {};
    vk::raii::PipelineLayout m_pipeline_layout = nullptr;
    vk::raii::DescriptorSetLayout m_descriptor_set_layout = nullptr;
    vk::raii::Pipeline m_graphics_pipeline = nullptr;
    vk::raii::CommandPool m_command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_command_buffers = {};
    vk::raii::DescriptorPool m_descriptor_pool = nullptr;
    std::vector<vk::raii::DescriptorSet> m_descriptor_sets = {};

    // Sync
    std::vector<vk::raii::Semaphore> m_present_complete_semaphores = {};
    std::vector<vk::raii::Semaphore> m_render_finished_semaphores = {};
    std::vector<vk::raii::Fence> in_flight_fences = {};

    // Memory allocator stuff
    vma::raii::Allocator m_allocator = nullptr;

    vma::raii::Buffer m_vertex_buffer = nullptr;
    vma::raii::Buffer m_index_buffer = nullptr;
    std::vector<vma::raii::Buffer> m_uniform_buffers = {};
    std::vector<void*> m_uniform_buffers_mapped = {};

    // Configuration
    std::vector<const char*> m_required_extensions;
    std::vector<const char*> m_validation_layers;
    bool m_enable_validation = true;
    vk::Flags<vk::DebugUtilsMessageSeverityFlagBitsEXT> m_validation_log_level;
    vk::Flags<vk::DebugUtilsMessageTypeFlagBitsEXT> m_validation_message_types;
    GpuPreference m_gpu_preference = GpuPreference::Discrete;
    float m_max_sampler_anisotropy = 1.0f;

    // Texture
    vma::raii::Image m_texture_image = nullptr;
    vk::raii::ImageView m_texture_image_view = nullptr;
    vk::raii::Sampler m_texture_sampler = nullptr;

    // Timers
    std::chrono::high_resolution_clock::time_point m_start_time;
    std::chrono::high_resolution_clock::time_point m_frame_start_time;
    float m_elapsed_time = 0.0f;
    float m_last_frame_time = 0.0f;

    // Depth
    vk::raii::Image m_depth_image = nullptr;
    vk::raii::ImageView m_depth_image_view = nullptr;

    auto init_vulkan() -> void;
    auto create_instance() -> void;
    auto setup_debug_messenger() -> void;
    auto pick_physical_device() -> void;
    auto get_physical_device_properties() -> void;
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
    auto create_descriptor_pool() -> void;
    auto create_descriptor_sets() -> void;
    auto create_image_sampler() -> void;
    auto create_depth_resources() -> void;

    auto load_assets() -> void;

    auto load_texture(vk::raii::CommandBuffer& command_buffer, const std::string& file_path)
        -> void;

    auto are_required_extensions_supported_by_instance() -> bool;
    auto are_required_validation_layers_supported_by_instance() -> bool;
    auto get_validation_layers() -> std::vector<const char*>;
    auto is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool;
    auto find_queue_families(const vk::raii::PhysicalDevice& device) -> FamilyIndices;
    auto choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;
    auto create_swapchain_image_views() -> void;
    auto find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties) -> uint32_t;
    auto transition_image_layout(
        vk::raii::CommandBuffer& command_buffer,
        const vk::Image& image,
        const ImageTransitionInfo& info
    ) -> void;
    auto find_supported_image_format(
        const std::vector<vk::Format>& candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features
    ) -> vk::Format;
    auto find_depth_format() -> vk::Format;

    auto copy_buffer_to_image(
        const vk::raii::CommandBuffer& command_buffer,
        const vma::raii::Buffer& buffer,
        vma::raii::Image& destination,
        uint32_t width,
        uint32_t height
    ) -> void;

    // per frame
    auto update_uniform_buffer(uint32_t current_image) -> void;

    [[nodiscard]] auto create_shader_module(const std::span<uint8_t>& code) const
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

    auto update_clocks_before_draw() -> void;
    auto update_clocks_after_draw() -> void;

    auto main_loop() -> void;

    auto cleanup() -> void;
    auto cleanup_swap_chain() -> void;
};
