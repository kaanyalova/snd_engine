#pragma once

#include <deque>
#include <functional>

class DeletionQueue {
  public:
    auto push_function(std::function<void()>&& function) -> void;
    auto flush() -> void;

  private:
    std::deque<std::function<void()>> m_deletors;
};