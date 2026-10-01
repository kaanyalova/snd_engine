#include "VulkanSampler.hpp"
VulkanSampler::VulkanSampler(VulkanRenderer& renderer, const SamplerData& sampler_data) : m_renderer(renderer) {
    create_sampler(sampler_data);
}

VulkanSampler::~VulkanSampler() {
    m_renderer.mark_sampler_slot_for_deletion(m_bound_slot);
}

auto VulkanSampler::create_sampler(const SamplerData& sampler_data) -> void {
    auto sampler_create_info = vk::SamplerCreateInfo {
        .magFilter = static_cast<vk::Filter>(sampler_data.mag_filter),
        .minFilter = static_cast<vk::Filter>(sampler_data.min_filter),
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = 8.0f,
        .maxLod = 1
    };
    m_sampler = m_renderer.get_device().inner().createSampler(sampler_create_info);
}
