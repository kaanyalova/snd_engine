#pragma once
#include <vulkan/vulkan.hpp>

struct ImageTransitionInfo {
    vk::ImageLayout source;
    vk::AccessFlags source_access_flags;
    vk::PipelineStageFlags source_stage_flags;

    vk::ImageLayout destination;
    vk::AccessFlags destination_accesss_flags;
    vk::PipelineStageFlags destination_stage_flags;

    static auto undefined_to_dst_optimal() -> ImageTransitionInfo;
    static auto dst_optimal_to_shader_optimal() -> ImageTransitionInfo;
    static auto undefined_to_depth_attachment_optimal() -> ImageTransitionInfo;
    static auto undefined_to_color_attachment_optimal() -> ImageTransitionInfo;
};
