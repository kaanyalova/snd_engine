#pragma once
#include <cstdint>
#include <span>

#include "../Image/Image.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"

class IRendererBackend {
  public:
    virtual auto create_texture(uint32_t handle) -> Texture;

    virtual auto create_sampler(uint32_t handle) -> Sampler;

    virtual auto destroy_sampler(uint32_t sampler_index) -> void;
};