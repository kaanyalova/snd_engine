#include "snd_core/Resources/Scene/SceneNode.hpp"

SceneNode::SceneNode() {
}
SceneNode::SceneNode(glm::mat4 transform) : m_transform(transform) {
}