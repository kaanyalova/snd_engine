#include "RenderGraph.hpp"

auto RenderGraph::get_color_attachment(const std::string_view name) -> std::optional<ColorAttachment> {
    const auto it = m_color_attachments.find(name);
    if (it == m_color_attachments.end()) {
        return std::nullopt;
    }
    return it->second;
}
