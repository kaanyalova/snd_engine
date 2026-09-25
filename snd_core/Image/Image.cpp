#include "Image.hpp"

auto Image::view() -> ImageView {
    return ImageView {
        .width = width,
        .height = height,
        .data = data,
        .format = format,
    };
}