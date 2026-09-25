#pragma once
#include "snd_core/Renderer/Vulkan/VulkanRenderer.hpp"
#include "snd_core/Resources/Scene/SceneData.hpp"

/**
 * A sampler that's allocated on the GPU and bound to one of the slots of the
 * descriptor set
 */
class VulkanSampler {
  public:
    VulkanSampler(VulkanRenderer& renderer, const SamplerData& sampler_data);

    ~VulkanSampler();

    auto inner() const -> const vk::Sampler& { return m_sampler; }
    auto set_slot(uint32_t slot) -> void { m_bound_slot = slot; }

  private:
    VulkanRenderer& m_renderer;
    vk::raii::Sampler m_sampler = nullptr;
    uint32_t m_bound_slot = 0;

    auto create_sampler(const SamplerData& sampler_data) -> void;
};
