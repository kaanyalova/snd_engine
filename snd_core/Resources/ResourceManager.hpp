#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "../SlotMap.hpp"
#include "./SceneData.hpp"
#include "GltfLoader.hpp"
#include "Material.hpp"

class ResourceManager {
  public:
    // auto add_mesh(std::span<Vertex> vertex_data, std::span<uint32_t> index_data) -> MeshHandle;

  private:
    SceneData scene_data;
    GltfLoader m_gltf_loader;
};