#pragma once

template <class... Ts>
struct match : Ts... {
    using Ts::operator()...;
};