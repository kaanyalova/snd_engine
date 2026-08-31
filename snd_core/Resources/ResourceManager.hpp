#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "Scene/SceneData.hpp"
#include "snd_core/Resources/GltfLoader.hpp"
#include "snd_core/Resources/Material.hpp"
#include "snd_core/SlotMap.hpp"

class ResourceManager {
  public:
    // auto add_mesh(std::span<Vertex> vertex_data, std::span<uint32_t> index_data) -> MeshHandle;

  private:
    // SceneData scene_data;
    GltfLoader m_gltf_loader;
};