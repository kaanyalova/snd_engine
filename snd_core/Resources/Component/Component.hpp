#pragma once
class Component {
  public:
    virtual auto get_draw_commands() -> void;
};