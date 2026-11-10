#pragma once
#include "snd_core/Renderer/Vulkan/Resources/Buffers/VulkanBuffer.hpp"
#include "snd_core/Renderer/Vulkan/Resources/VulkanImage.hpp"

class RenderPass {
  public:
    virtual auto setup() -> void = 0;
    virtual auto execute() -> void = 0;

    virtual ~RenderPass() = default;

  protected:
    enum class AttachmentFlags : uint8_t {
        Read = 1 << 0,
        Write = 1 << 1,
    };

    auto set_shader(std::string_view name) -> void;
    auto bind_scene() -> void;
    auto create_color_attachment(const std::string& name, AttachmentFlags flag) -> void;
    auto add_color_attachment(std::string_view name, AttachmentFlags flags) -> void;
    auto add_buffer_attachment(std::string_view name, AttachmentFlags flags) -> void;

    class ExecuteContext {};
};
