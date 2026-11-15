#include "RenderPass.hpp"

#include "RenderGraph.hpp"
RenderPass::RenderPassContext::RenderPassContext(VulkanRenderer& renderer, RenderGraph& render_graph)
    : m_renderer(renderer), m_render_graph(render_graph) {
}
auto RenderPass::RenderPassContext::get_color_attachment(std::string_view name) -> std::optional<ColorAttachment> {
    return m_render_graph.get_color_attachment(name);
}
