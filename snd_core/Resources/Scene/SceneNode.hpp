#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "snd_core/Resources/Component/ModelComponent.hpp"

class SceneNode {
  public:
    SceneNode();
    explicit SceneNode(glm::mat4 transform);

    auto get_children() const -> const std::vector<std::unique_ptr<SceneNode>>& { return m_children; }
    auto get_mutable_children() -> std::vector<std::unique_ptr<SceneNode>>& { return m_children; }

    auto get_model_components() const -> const std::vector<ModelComponent>& { return m_model_components; }
    auto add_model_component(ModelComponent&& model_component) -> void {
        m_model_components.emplace_back(std::move(model_component));
    }

    auto get_parent() const -> const SceneNode* { return m_parent; }
    auto has_parent() const -> bool { return m_parent != nullptr; }

    auto has_children() const -> bool { return m_children.size() > 0; }
    auto add_child(SceneNode&& child) -> void {
        m_children.emplace_back(std::make_unique<SceneNode>(std::move(child)));
    }
    auto child_count() const -> uint32_t { return m_children.size(); }

    auto set_transform(const glm::mat4& transform) -> void { m_transform = transform; }
    auto get_transform() const -> const glm::mat4& { return m_transform; }

    auto set_baked_transform_index(size_t index) -> void { m_baked_transform_index = index; }
    auto is_baked() const -> bool { return m_baked_transform_index.has_value(); }
    auto get_baked_transform_index() const -> uint32_t { return m_baked_transform_index.value(); }

  private:
    SceneNode* m_parent = nullptr;
    std::vector<std::unique_ptr<SceneNode>> m_children = {};

    std::vector<ModelComponent> m_model_components = {};

    glm::mat4 m_transform = glm::mat4(1.0f);

    glm::mat4 m_baked_transform = glm::mat4(1.0f);
    std::optional<uint32_t> m_baked_transform_index = std::nullopt;
};