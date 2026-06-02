#include "ImageTransitionInfo.hpp"

#include "vulkan/vulkan.hpp"

auto ImageTransitionInfo::dst_optimal_to_shader_optimal() -> ImageTransitionInfo {
    return ImageTransitionInfo {
        .source = vk::ImageLayout::eTransferDstOptimal,
        .source_access_flags = vk::AccessFlagBits::eTransferWrite,
        .source_stage_flags = vk::PipelineStageFlagBits::eTransfer,

        .destination = vk::ImageLayout::eShaderReadOnlyOptimal,
        .destination_accesss_flags = vk::AccessFlagBits::eShaderRead,
        .destination_stage_flags = vk::PipelineStageFlagBits::eFragmentShader,
    };
}

auto ImageTransitionInfo::undefined_to_dst_optimal() -> ImageTransitionInfo {
    return ImageTransitionInfo {
        .source = vk::ImageLayout::eUndefined,
        .source_access_flags = {},
        .source_stage_flags = vk::PipelineStageFlagBits::eTopOfPipe,

        .destination = vk::ImageLayout::eTransferDstOptimal,
        .destination_accesss_flags = vk::AccessFlagBits::eTransferWrite,
        .destination_stage_flags = vk::PipelineStageFlagBits::eTransfer,
    };
}

auto ImageTransitionInfo::undefined_to_depth_attachment_optimal() -> ImageTransitionInfo {
    return ImageTransitionInfo {
        .source = vk::ImageLayout::eUndefined,
        .source_access_flags = vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        .source_stage_flags = vk::PipelineStageFlagBits::eEarlyFragmentTests |
                              vk::PipelineStageFlagBits::eLateFragmentTests,

        .destination = vk::ImageLayout::eDepthAttachmentOptimal,
        .destination_accesss_flags = vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        .destination_stage_flags = vk::PipelineStageFlagBits::eEarlyFragmentTests |
                                   vk::PipelineStageFlagBits::eLateFragmentTests,
    };
}

auto ImageTransitionInfo::undefined_to_color_attachment_optimal() -> ImageTransitionInfo {
    return ImageTransitionInfo {
        .source = vk::ImageLayout::eUndefined,
        .source_access_flags = {},
        .source_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,

        .destination = vk::ImageLayout::eColorAttachmentOptimal,
        .destination_accesss_flags = vk::AccessFlagBits::eColorAttachmentWrite,
        .destination_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };
}
