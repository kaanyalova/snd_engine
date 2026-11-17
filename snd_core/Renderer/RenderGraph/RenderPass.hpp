#pragma once

#include "BufferAttachment.hpp"
#include "RenderGraph.hpp"
#include "snd_core/Renderer/RenderGraph/ColorAttachment.hpp"
#include "snd_core/Renderer/Vulkan/VulkanBuffer.hpp"
#include "snd_core/Renderer/Vulkan/VulkanImage.hpp"

class RenderPass {
  public:
    virtual ~RenderPass() = default;

  private:
    enum class AccessFlags : uint8_t {
        Read = 1 << 0,
        Write = 1 << 1,
    };

    class Builder {
        auto with_shader(std::string_view shader_name) -> Builder;
        
        // color attachments are bound to VulkanRenderer::DescriptorSetBindings::Transient which is 2
        auto with_new_color_attachment(
            std::string_view name, uint32_t binding, AccessFlags flags, ColorAttachment attachment
        ) -> Builder&;
        auto with_exiting_color_attachment(std::string_view name, AccessFlags flags, uint32_t binding) -> Builder&;

        // Buffers are bound to the buffer referenced by the push constant
        auto with_new_buffer(std::string_view name, AccessFlags flags, BufferAttachment attachment) -> Builder&;
        auto with_existing_buffer(std::string_view name, AccessFlags flags) -> Builder&;

        auto build() -> RenderPass;
    };

    auto set_shader(std::string_view name) -> void;
    auto bind_scene() -> void;
    auto add_color_attachment(std::string_view name, AccessFlags flags) -> void;
    auto add_buffer_attachment(std::string_view name, AccessFlags flags) -> void;

    class RenderPassContext {
      public:
        RenderPassContext(VulkanRenderer& renderer, RenderGraph& render_pass);
        auto get_color_attachment(std::string_view name) -> std::optional<ColorAttachment>;
        auto get_renderer() -> VulkanRenderer&;

      private:
        VulkanRenderer& m_renderer;
        RenderGraph& m_render_graph;
    };
};
