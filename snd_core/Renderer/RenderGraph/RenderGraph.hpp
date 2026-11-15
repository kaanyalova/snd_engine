#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ColorAttachment.hpp"
#include "snd_core/Utils/StringUtils.hpp"

class RenderGraph {
  public:
    auto new_pass() -> void;
    auto get_color_attachment(std::string_view name) -> std::optional<ColorAttachment>;

  private:
    std::unordered_map<std::string, ColorAttachment, StringUtils::TransparentStringHash, std::equal_to<>>
        m_color_attachments;
};
