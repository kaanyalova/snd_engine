#include <algorithm>
#include <print>
#include <ranges>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"

VulkanDevice::VulkanDevice(const DeviceCreationInfo& info, Window& window) : m_info(info) {
    create_instance();
    // if (m_enable_validation) {
    //     setup_debug_messenger();
    // }
    create_physical_device();
    get_physical_device_properties();
    create_logical_device();
    create_memory_allocator();
    find_depth_format();
}

auto VulkanDevice::create_instance() -> void {
    auto constexpr application_info = vk::ApplicationInfo {
        .pApplicationName = "vulkan",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    for (const char* extension : m_info.sdl_required_extensions) {
        std::println("sdl required extension: {}", extension);
    }

    m_required_extensions.insert(
        m_required_extensions.end(),
        m_info.sdl_required_extensions.begin(),
        m_info.sdl_required_extensions.end()
    );

    // m_required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);

    if (m_info.enable_validation) {
        m_required_extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    if (!are_required_extensions_supported_by_instance()) {
        throw std::runtime_error("some of the required extensions are not supported for vulkan");
    }

    std::vector<const char*> enabled_validation_layers = {};

    if (m_info.enable_validation) {
        enabled_validation_layers = m_validation_layers;
    }

    auto create_info = vk::InstanceCreateInfo {
        //.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_validation_layers.size()),
        .ppEnabledLayerNames = enabled_validation_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(m_required_extensions.size()),
        .ppEnabledExtensionNames = m_required_extensions.data(),

    };

    m_instance = vk::raii::Instance(m_vulkan_context, create_info);
}

/**
 * @brief check if all the extensions in m_required_extensions are supported by the instance
 *
 * @return true if all extensions are supported, false otherwise
 */
auto VulkanDevice::are_required_extensions_supported_by_instance() -> bool {
    std::vector<vk::ExtensionProperties> extension_properties =
        m_vulkan_context.enumerateInstanceExtensionProperties();

    return std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
        return std::ranges::any_of(
            extension_properties, [&](const vk::ExtensionProperties& available_extension) -> bool {
                return std::strcmp(required_extension, available_extension.extensionName) == 0;
            }
        );
    });
}

auto VulkanDevice::create_physical_device() -> void {
    std::vector<vk::raii::PhysicalDevice> physical_devices = m_instance.enumeratePhysicalDevices();

    if (m_info.gpu_preference == GpuPreference::Discrete) {
        const auto discrete_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                       is_device_suitable(device);
            });

        if (discrete_gpu_it != physical_devices.end()) {
            m_physical_device = *discrete_gpu_it;
            return;
        }
    } else if (m_info.gpu_preference == GpuPreference::Integrated) {
        const auto integrated_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType ==
                           vk::PhysicalDeviceType::eIntegratedGpu &&
                       is_device_suitable(device);
            });

        if (integrated_gpu_it != physical_devices.end()) {
            m_physical_device = *integrated_gpu_it;
            return;
        }
    }

    // cannot find the preferred gpu, fallback to any one that works
    auto fallback_it =
        std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
            return is_device_suitable(device);
        });

    if (fallback_it != physical_devices.end()) {
        m_physical_device = *fallback_it;
        return;
    }

    // no gpus are suitable
    throw std::runtime_error("failed to find a suitable GPU");
}

/**
 * @brief Check if the PhysicalDevice is suitable by checking if it supports the required features,
 * extensions and queue families
 *
 * @param device the physical device to check
 * @return true if the device is suitable, false otherwise
 */
auto VulkanDevice::is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool {
    const vk::PhysicalDeviceProperties device_properties = device.getProperties();
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();
    std::vector<vk::ExtensionProperties> device_extension_properties =
        device.enumerateDeviceExtensionProperties();

    const bool is_api_version_suitable = device_properties.apiVersion >= vk::ApiVersion13;
    const bool has_graphics_queue = std::ranges::any_of(
        queue_family_properties, [](const vk::QueueFamilyProperties& properties) {
            return static_cast<bool>(properties.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    // TODO
    // const bool supports_required_extensions =
    //     std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
    //         return std::ranges::any_of(
    //             device_extension_properties,
    //             [&](const vk::ExtensionProperties& available_extension) {
    //                 return std::strcmp(available_extension.extensionName, required_extension) ==
    //                 0;
    //             }
    //         );
    //     });

    const bool is_suitable =
        is_api_version_suitable && has_graphics_queue;  // && supports_required_extensions;

    return is_suitable;
}

auto VulkanDevice::get_physical_device_properties() -> void {
    vk::PhysicalDeviceProperties properties = m_physical_device.getProperties();
    m_max_sampler_anisotropy = properties.limits.maxSamplerAnisotropy;
}

auto VulkanDevice::create_logical_device() -> void {
    float queue_priority = 1.0f;

    auto device_queue_create_info = vk::DeviceQueueCreateInfo {
        .queueFamilyIndex = m_family_indices.graphics,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    using DeviceFeaturesChain = vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>;

    auto feature_chain = DeviceFeaturesChain {
        vk::PhysicalDeviceFeatures2 {
            .features =
                vk::PhysicalDeviceFeatures {
                    .samplerAnisotropy = vk::True,
                },
        },
        vk::PhysicalDeviceVulkan11Features {.shaderDrawParameters = vk::True},
        vk::PhysicalDeviceVulkan13Features {
            .synchronization2 = vk::True, .dynamicRendering = vk::True
        },
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {.extendedDynamicState = vk::True},
    };

    std::vector<const char*> device_extensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRSwapchainExtensionName,
        vk::EXTDescriptorHeapExtensionName,
    };

    auto device_create_info = vk::DeviceCreateInfo {
        .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
    };

    m_device = vk::raii::Device(m_physical_device, device_create_info);
    m_queues.graphics = vk::raii::Queue(m_device, m_family_indices.graphics, 0);
    m_queues.presentation = vk::raii::Queue(m_device, m_family_indices.presentation, 0);
}

auto VulkanDevice::find_family_indices() -> void {
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        m_physical_device.getQueueFamilyProperties();

    const auto graphics_queue_family_property = std::ranges::find_if(
        queue_family_properties, [](const vk::QueueFamilyProperties& qfp) -> bool {
            return static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
        }
    );

    if (graphics_queue_family_property == queue_family_properties.end()) {
        throw std::runtime_error("failed to find a graphics queue family");
    }

    auto graphics_family_index = static_cast<uint32_t>(
        std::distance(queue_family_properties.begin(), graphics_queue_family_property)
    );

    // check if the graphics family also supports presentation
    bool also_supports_presentation =
        m_physical_device.getSurfaceSupportKHR(graphics_family_index, *m_surface);

    uint32_t presentation_family_index = 0;

    if (also_supports_presentation) {
        presentation_family_index = graphics_family_index;
    } else {
        auto iota = std::views::iota(0u, static_cast<uint32_t>(queue_family_properties.size()));
        auto presentation_family_it = std::ranges::find_if(iota, [&](uint32_t index) -> bool {
            return m_physical_device.getSurfaceSupportKHR(index, *m_surface);
        });

        if (presentation_family_it == iota.end()) {
            throw std::runtime_error("failed to find a presentation queue family");
        }

        presentation_family_index = *presentation_family_it;
    }

    auto family_indices = FamilyIndices {
        .graphics = graphics_family_index,
        .presentation = presentation_family_index,
    };
}

auto VulkanDevice::create_memory_allocator() -> void {
    auto vma_allocator_create_info = vma::AllocatorCreateInfo {
        .physicalDevice = *m_physical_device,
        .vulkanApiVersion = vk::ApiVersion14,
    };

    m_allocator = vma::raii::createAllocator(m_instance, m_device, vma_allocator_create_info);
}

auto VulkanDevice::get_device_address(const vk::Buffer& buffer) -> vk::DeviceAddress {
    auto device_address_info = vk::BufferDeviceAddressInfo {
        .buffer = buffer,
    };

    return m_device.getBufferAddress(device_address_info);
}

auto VulkanDevice::find_depth_format() -> void {
    std::vector<vk::Format> depth_formats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
    };

    auto depth_format = std::ranges::find_if(depth_formats, [&](vk::Format format) -> bool {
        vk::FormatProperties2 format_properties = m_physical_device.getFormatProperties2(format);
        return static_cast<bool>(
            format_properties.formatProperties.optimalTilingFeatures &
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    });

    if (depth_format == depth_formats.end()) {
        throw std::runtime_error("cannot find depth format");
    }

    m_depth_format = *depth_format;
}
