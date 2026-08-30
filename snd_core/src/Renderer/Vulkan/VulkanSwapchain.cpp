
#include "VulkanSwapchain.hpp"

#include "vk_mem_alloc_structs.hpp"
#include "vulkan/vulkan.hpp"

VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, Window& window)
    : m_device(device), m_window(window) {
    create();
    create_image_views();
    create_depth_images();
}

auto VulkanSwapchain::create() -> void {
    auto surface_capabilities =
        m_device.get_physical_device().getSurfaceCapabilitiesKHR(m_device.get_surface());

    m_extent = choose_extent(surface_capabilities);

    std::vector<vk::SurfaceFormatKHR> surface_formats =
        m_device.get_physical_device().getSurfaceFormatsKHR(m_device.get_surface());
    m_surface_format = choose_surface_format(surface_formats);

    uint32_t min_image_count = std::max(3u, surface_capabilities.minImageCount);

    // clamp it between the surface's capabilities
    if (surface_capabilities.maxImageCount > 0) {
        min_image_count = std::clamp(
            min_image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount
        );
    }

    uint32_t image_count = surface_capabilities.minImageCount + 1;

    if (surface_capabilities.maxImageCount > 0 &&
        image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }

    vk::PresentModeKHR present_mode = choose_present_mode(
        m_device.get_physical_device().getSurfacePresentModesKHR(*m_device.get_surface())
    );

    auto swap_chain_create_info = vk::SwapchainCreateInfoKHR {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = *m_device.get_surface(),
        .minImageCount = min_image_count,
        .imageFormat = m_surface_format.format,
        .imageColorSpace = m_surface_format.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = true,
        .oldSwapchain = nullptr,
    };

    std::array<uint32_t, 2> family_indices = {
        m_device.get_family_indices().graphics,
        m_device.get_family_indices().presentation,
    };

    if (m_device.get_family_indices().graphics != m_device.get_family_indices().presentation) {
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        // todo check if this cast actually works
        swap_chain_create_info.pQueueFamilyIndices = family_indices.data();
    }

    m_swapchain = vk::raii::SwapchainKHR(m_device.get_device(), swap_chain_create_info);
    m_images = m_swapchain.getImages();
}

auto VulkanSwapchain::choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities)
    -> vk::Extent2D {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    auto [width, height] = m_window.size();

    return vk::Extent2D {
        .width = std::clamp<uint32_t>(
            width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
        ),
        .height = std::clamp<uint32_t>(
            height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
        ),
    };
}

/**
 * @brief Choose the swap surface format based on the available formats
 *
 * @param available_formats list of available formats by the PhysicalDevice
 * @return vk::SurfaceFormatKHR the chosen surface format
 */
auto VulkanSwapchain::choose_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats
) -> vk::SurfaceFormatKHR {
    auto available_format_it =
        std::ranges::find_if(available_formats, [](const vk::SurfaceFormatKHR& available_format) {
            return available_format.format == vk::Format::eB8G8R8A8Srgb &&
                   available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    if (available_format_it != available_formats.end()) {
        return *available_format_it;
    }

    // return the first one if the preferred one is not found
    return available_formats[0];
}

auto VulkanSwapchain::choose_present_mode(const std::vector<vk::PresentModeKHR>& available_modes)
    -> vk::PresentModeKHR {
    auto mailbox_mode_it = std::ranges::find(available_modes, vk::PresentModeKHR::eMailbox);

    if (mailbox_mode_it != available_modes.end()) {
        return *mailbox_mode_it;
    }

    return vk::PresentModeKHR::eFifo;
}

auto VulkanSwapchain::create_image_views() -> void {
    m_image_views.clear();

    auto sub_resource_range = vk::ImageSubresourceRange {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    auto image_view_create_info = vk::ImageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = m_surface_format.format,
        .subresourceRange = sub_resource_range,
    };

    for (vk::Image image : m_images) {
        image_view_create_info.image = image;
        m_image_views.emplace_back(m_device.get_device(), image_view_create_info);
    }
}

auto VulkanSwapchain::create_depth_images() -> void {
    std::pair<uint32_t, uint32_t> window_size = m_window.size();

    auto image_create_info = vk::ImageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = m_device.get_depth_format(),
        .extent = vk::Extent3D {
            .width = window_size.first,
            .height = window_size.second,
            .depth = 1,
        },

    };

    auto vma_allocation_create_info = vma::AllocationCreateInfo {
        .flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
        .usage = vma::MemoryUsage::eAuto,
    };

    m_depth_image = m_device.get_allocator().createImage(
        image_create_info, vma_allocation_create_info, nullptr
    );

    auto depth_image_view_create_info = vk::ImageViewCreateInfo {
        .image = m_depth_image,
        .viewType = vk::ImageViewType::e2D,
        .format = m_device.get_depth_format(),
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
        }
    };

    m_depth_image_view = m_device.get_device().createImageView(depth_image_view_create_info);
}
