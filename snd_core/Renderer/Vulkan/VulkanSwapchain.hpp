#pragma once

#include <cstdint>

#include "VulkanCommandBuffer.hpp"
#include "VulkanDevice.hpp"
#include "vk_mem_alloc_raii.hpp"

class VulkanSwapchain {
  public:
    VulkanSwapchain(VulkanDevice& device, Window& window);

    const static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    auto get_swapchain() -> vk::raii::SwapchainKHR& { return m_swapchain; }
    auto get_images() -> std::vector<vk::Image>& { return m_images; }
    auto get_image_views() -> std::vector<vk::raii::ImageView>& { return m_image_views; }
    auto get_surface_format() -> vk::SurfaceFormatKHR& { return m_surface_format; }

  private:
    Window& m_window;
    VulkanDevice& m_device;

    vk::raii::SwapchainKHR m_swapchain = nullptr;
    std::vector<vk::Image> m_images = {};
    std::vector<vk::raii::ImageView> m_image_views = {};
    std::vector<CommandBuffer> m_command_buffers = {};
    vk::Extent2D m_extent;
    vk::SurfaceFormatKHR m_surface_format;
    vma::raii::Image m_depth_image = nullptr;
    vk::raii::ImageView m_depth_image_view = nullptr;

    // Sync
    std::vector<vk::raii::Semaphore> m_present_complete_semaphores = {};
    std::vector<vk::raii::Semaphore> m_render_finished_semaphores = {};
    std::vector<vk::raii::Fence> m_in_flight_fences = {};

    auto create() -> void;
    auto create_image_views() -> void;
    auto create_depth_images() -> void;

    auto choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D;
    auto choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
        -> vk::SurfaceFormatKHR;
    auto choose_present_mode(const std::vector<vk::PresentModeKHR>& available_modes)
        -> vk::PresentModeKHR;
};
