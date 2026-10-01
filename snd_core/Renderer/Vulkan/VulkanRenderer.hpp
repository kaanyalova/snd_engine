#pragma once

#include <imgui_impl_vulkan.h>

#include <optional>

#include "../../Resources/Scene/SceneData.hpp"
#include "Resources/VulkanImage.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Renderer/Vulkan/VulkanSwapchain.hpp"

struct RendererValidationSettings {};

struct RendererSettings {
    bool enable_validation = true;
    GpuPreference gpu_preference = GpuPreference::Integrated;
};

class VulkanRenderer {
  public:
    VulkanRenderer(Window& window, const RendererSettings& settings);

    using CommandBufferRecordLambda = std::function<void(vk::raii::CommandBuffer& command_buffer)>;

    auto recreate_swapchain() -> void;
    auto create_imgui_init_info() -> ImGui_ImplVulkan_InitInfo;
    auto process() -> void;

    auto get_device() -> VulkanDevice& { return m_device.value(); }
    auto get_swapchain() -> VulkanSwapchain& { return m_swapchain.value(); }
    auto get_imgui_descriptor_pool() -> const vk::DescriptorPool& { return *m_imgui_descriptor_pool; }
    auto get_renderer_descriptor_pool() -> const vk::DescriptorPool& { return *m_renderer_descriptor_pool; }
    auto get_renderer_descriptor_set() -> const vk::DescriptorSet& { return *m_renderer_descriptor_set; }

    auto get_gpu_maximum_descriptor_set_count() -> uint32_t;
    static auto convert_to_vulkan_image_format(ImageFormat imageFormat) -> vk::Format;

    auto submit_single_command(CommandBufferRecordLambda&& record, vk::raii::Fence* fence) -> void;
    auto push_per_frame_commands(CommandBufferRecordLambda&& record) -> void;

    auto mark_image_slot_for_deletion(uint32_t slot) -> void;
    auto mark_sampler_slot_for_deletion(uint32_t slot) -> void;

    auto get_next_free_image_slots(uint32_t count) -> uint32_t;
    auto get_next_free_sampler_slots(uint32_t count) -> uint32_t;

    enum class DescriptorSetBindings : uint8_t {
        SampledImages = 0,
        Samplers = 1,
    };

    // auto allocate_samplers(std::span<SamplerData> samplers) -> std::vector<>;

  private:
    static constexpr uint32_t DESCRIPTOR_POOL_MAX_SAMPLED_IMAGE_COUNT = 1'000'000;
    static constexpr uint32_t DESCRIPTOR_POOL_MAX_SAMPLER_COUNT = 1'000;

    // we might want to start out the allocated sets from the descriptor at a lower value
    // then allocate more as needed, this increases the complexity and is not necessary for now
    // though
    //
    // uint32_t m_gpu_max_descriptor_set_count = 0;
    // static constexpr uint32_t DESCRIPTOR_SET_INITIAL_COUNT = 1024;
    // static constexpr float DESCRIPTOR_SET_IMAGES_REALLOCATE_SCALING_FACTOR = 1.5;

    std::optional<VulkanDevice> m_device = std::nullopt;
    std::optional<VulkanSwapchain> m_swapchain = std::nullopt;
    vk::raii::DescriptorPool m_imgui_descriptor_pool = nullptr;
    vk::raii::DescriptorPool m_renderer_descriptor_pool = nullptr;

    vk::raii::DescriptorSetLayout m_renderer_descriptor_set_layout = nullptr;
    vk::raii::DescriptorSet m_renderer_descriptor_set = nullptr;

    std::vector<CommandBufferRecordLambda> m_remaining_records;

    RendererSettings m_settings;

    Window& m_window;

    std::vector<uint32_t> m_image_slots_to_be_deleted;
    std::vector<uint32_t> m_sampler_slots_to_be_deleted;

    auto create_device() -> void;
    auto create_swapchain() -> void;

    auto create_imgui_descriptor_pool() -> void;

    auto create_renderer_descriptor_pool() -> void;
    auto create_renderer_descriptor_set_layout() -> void;
    auto allocate_renderer_descriptor_sets() -> void;
};
