#pragma once
#include <cstdint>
#include <span>

#include "snd_core/Image/Image.hpp"
#include "snd_core/Renderer/Sampler.hpp"
#include "snd_core/Renderer/Texture.hpp"

class IRendererBackend {
  public:
    virtual auto create_texture(uint32_t handle) -> Texture;

    virtual auto create_sampler(uint32_t handle) -> Sampler;

    virtual auto destroy_sampler(uint32_t sampler_index) -> void;
};