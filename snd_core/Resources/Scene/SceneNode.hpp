#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "snd_core/Resources/Component/Component.hpp"

class SceneNode {
  public:
    SceneNode();
    auto get_children() const -> const std::vector<SceneNode>& { return children; }
    auto get_components() const -> const std::vector<std::unique_ptr<Component>>& { return components; }

  private:
    SceneNode* parent;
    std::vector<SceneNode> children;
    std::vector<std::unique_ptr<Component>> components;
    glm::mat4 transform = glm::mat4(1.0f);
};