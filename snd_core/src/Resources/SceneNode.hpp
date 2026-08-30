#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Component/Component.hpp"

class SceneNode {
  public:
    SceneNode();

  private:
    SceneNode* parent;
    std::vector<SceneNode> children;
    std::vector<std::unique_ptr<Component>> components;
    glm::mat4 transform = glm::mat4(1.0f);
};