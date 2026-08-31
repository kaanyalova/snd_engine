#include <cstdint>
#include <optional>
#include <vector>

#include "SceneData.hpp"
#include "snd_core/Image/Image.hpp"
#include "snd_core/Renderer/Vulkan/VulkanScene.hpp"
#include "snd_core/Resources/Scene/SceneNode.hpp"

enum class SceneLoadState : uint8_t {
    Unloaded,
    LoadedOnCpu,
    LoadedOnGpu,
};

class Scene {
  public:
    Scene() = default;

    auto get_data() -> const SceneData& { return m_data; }

    auto push_material(MaterialData&& material) -> void;
    auto push_texture(Texture&& texture, Image&& image) -> void;
    auto push_primitive(std::vector<Vertex>&& vertices) -> void;
    auto push_primitive_instance(PrimitiveInstance&& instance) -> void;

    auto push_scene_data(SceneData&& other) -> void;
    auto push_scene(Scene&& other) -> void;

    auto get_root_node() -> SceneNode&;

  private:
    SceneNode root = SceneNode();
    SceneData m_data;
    PushOffsets m_push_offsets;

    std::optional<VulkanScene> m_vulkan_scene = std::nullopt;
    SceneLoadState m_scene_load_state = SceneLoadState::Unloaded;
};

auto Scene::push_material(MaterialData&& material) -> void {
}
