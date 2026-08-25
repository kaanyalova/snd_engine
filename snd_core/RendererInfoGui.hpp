#pragma once

class RendererInfoGui {
  public:
    static auto process() -> void;
    static auto set_frametime(float frame_time) -> void;

  private:
    static inline float m_frame_time = 0.0f;
};