#include "DeletionQueue.hpp"

auto DeletionQueue::push_function(std::function<void()>&& function) -> void {
    m_deletors.push_front(std::move(function));
}

auto DeletionQueue::flush() -> void {
    for (const auto& function : m_deletors) {
        function();
    }
    m_deletors.clear();
}
