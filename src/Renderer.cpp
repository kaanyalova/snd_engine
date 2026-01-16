module;
#include <SDL3/SDL.h>

#include <utility>
module Renderer;

import std;
import std.compat;
import vulkan;

Renderer::Renderer(CreateSurfaceFunc create_surface, RendererSettings settings)
    : m_required_extensions(std::move(settings.required_extensions))
    , m_validation_layers(std::move(settings.validation_layers))
    , m_enable_validation(settings.enable_validation)
    , m_validation_log_level(settings.validation_log_level)
    , m_validation_message_types(settings.validation_message_types)
    , m_prefer_discrete_gpu(settings.prefer_discrete_gpu)
    , m_create_surface_func(std::move(create_surface)) {
}

auto Renderer::init_vulkan() -> void {
    create_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
}

auto Renderer::create_instance() -> void {
    auto constexpr application_info = vk::ApplicationInfo {
        .pApplicationName = "vulkan",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    m_required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);

    bool extensions_supported = are_required_extensions_supported_by_instance();

    if (!extensions_supported) {
        throw std::runtime_error("some sdl3 extensions are not supported for vulkan");
    }

    std::vector<const char*> validation_layers = get_validation_layers();

    auto create_info = vk::InstanceCreateInfo {
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
        .ppEnabledLayerNames = validation_layers.data(),
        .enabledExtensionCount = m_required_extensions.size(),
        .ppEnabledExtensionNames = m_required_extensions.data(),

    };

    m_instance = vk::raii::Instance(m_context, create_info);
}

auto Renderer::are_required_extensions_supported_by_instance() -> bool {
    std::vector<vk::ExtensionProperties> extension_properties =
        m_context.enumerateInstanceExtensionProperties();

    return std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
        return std::ranges::any_of(
            extension_properties,
            [&](const vk::ExtensionProperties& available_extension) -> bool {
                return std::strcmp(required_extension, available_extension.extensionName) == 0;
            }
        );
    });
}

auto Renderer::are_required_validation_layers_supported_by_instance() -> bool {
    std::vector<vk::LayerProperties> layer_properties =
        m_context.enumerateInstanceLayerProperties();

    return std::ranges::all_of(m_validation_layers, [&](const char* required_layer) -> bool {
        return std::ranges::any_of(
            layer_properties,
            [&](const vk::LayerProperties& available_layer) -> bool {
                return std::strcmp(required_layer, available_layer.layerName) == 0;
            }
        );
    });
}

auto Renderer::get_validation_layers() -> std::vector<const char*> {
    if (m_enable_validation) {
        if (!are_required_extensions_supported_by_instance()) {
            throw std::runtime_error("some validation layers are not supported");
        }
        return std::move(m_validation_layers);
    }
    return {};
}

auto Renderer::is_device_suitable(const vk::raii::PhysicalDevice& device) -> bool {
    const vk::PhysicalDeviceProperties device_properties = device.getProperties();
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();
    std::vector<vk::ExtensionProperties> device_extension_properties =
        device.enumerateDeviceExtensionProperties();

    const bool is_api_version_suitable = device_properties.apiVersion >= vk::ApiVersion13;
    const bool has_graphics_queue = std::ranges::any_of(
        queue_family_properties,
        [](const vk::QueueFamilyProperties& properties) {
            return properties.queueFlags == vk::QueueFlagBits::eGraphics;
        }
    );
    const bool supports_required_extensions =
        std::ranges::all_of(m_required_extensions, [&](const char* required_extension) -> bool {
            return std::ranges::any_of(
                device_extension_properties,
                [&](const vk::ExtensionProperties& available_extension) {
                    return std::strcmp(available_extension.extensionName, required_extension) == 0;
                }
            );
        });

    const bool is_suitable =
        is_api_version_suitable && has_graphics_queue && supports_required_extensions;

    return is_suitable;
}

auto Renderer::find_queue_families(const vk::raii::PhysicalDevice& device) -> uint32_t {
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        m_physical_device.getQueueFamilyProperties();

    const auto graphics_queue_family_property = std::find_if(
        queue_family_properties.begin(),
        queue_family_properties.end(),
        [](vk::QueueFamilyProperties const& qfp) {
            return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
        }
    );

    return static_cast<uint32_t>(
        std::distance(queue_family_properties.begin(), graphics_queue_family_property)
    );
}

auto Renderer::setup_debug_messenger() -> void {
    auto messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT {
        .messageSeverity = m_validation_log_level,
        .messageType = m_validation_message_types,
        .pfnUserCallback = &debug_callback,
    };

    m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(messenger_create_info);
}

auto Renderer::pick_physical_device() -> void {
    std::vector<vk::raii::PhysicalDevice> physical_devices = m_instance.enumeratePhysicalDevices();

    if (m_prefer_discrete_gpu) {
        const auto discrete_gpu_it =
            std::ranges::find_if(physical_devices, [&](const vk::raii::PhysicalDevice& device) {
                return device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
                       is_device_suitable(device);
            });

        if (discrete_gpu_it != physical_devices.end()) {
            m_physical_device = *discrete_gpu_it;
            return;
        }
    } else {
        // pick igpu over any gpu if m_prefer_discrete_gpu is false
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

auto Renderer::create_logical_device() -> void {
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        m_physical_device.getQueueFamilyProperties();

    uint32_t graphics_queue_family_index = find_queue_families(m_physical_device);
    float queue_priority = 1.0f;

    auto device_queue_create_info = vk::DeviceQueueCreateInfo {
        .queueFamilyIndex = graphics_queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    using DeviceFeaturesChain = vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>;

    auto feature_chain = DeviceFeaturesChain {
        vk::PhysicalDeviceFeatures2 {},
        vk::PhysicalDeviceVulkan13Features {.dynamicRendering = true},
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {.extendedDynamicState = true},
    };

    std::vector<const char*> device_extensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
    };

    auto device_create_info = vk::DeviceCreateInfo {
        .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
    };

    m_device = vk::raii::Device(m_physical_device, device_create_info);
    m_graphics_queue = vk::raii::Queue(m_device, graphics_queue_family_index, 0);
}

auto Renderer::create_surface() -> void {
    vk::SurfaceKHR surface = m_create_surface_func(m_instance);
    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}

auto Renderer::debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
    void* _data
) -> vk::Bool32 {
    std::println("validation layer: {}", callback_data->pMessage);
    return vk::False;
}

auto Renderer::run() -> void {
}

auto Renderer::main_loop() -> void {
}
