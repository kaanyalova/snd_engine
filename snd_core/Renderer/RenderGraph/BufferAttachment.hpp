#pragma once

#include <cstdint>
#include <span>

struct BufferAttachment {
    std::span<uint8_t> data;
};