#pragma once

#include <type_traits>
#include <variant>

template <typename T>
concept SomeConcept = true;

template <typename>
struct IsLegitVariant : std::false_type {};

template <typename... Ts>
struct IsLegitVariant<std::variant<Ts...>> {
    constexpr static bool value = (SomeConcept<Ts> && ...);
};

template <typename T>
constexpr bool is_legit_variant_v = IsLegitVariant<T>::value;

template <typename V>
concept LegitVariant = is_legit_variant_v<V>;