#pragma once

#include <cstddef>
#include <type_traits>

namespace bengal::detail {

template <typename... Ts>
struct are_unique : std::true_type {};

template <typename T, typename... Ts>
struct are_unique<T, Ts...>
    : std::bool_constant<(!std::is_same_v<T, Ts> && ...) &&
                         are_unique<Ts...>::value> {};

template <typename... Ts>
inline constexpr bool are_unique_v = are_unique<Ts...>::value;

template <typename T, typename... Ts>
inline constexpr bool contains_type_v = (std::is_same_v<T, Ts> || ...);

template <typename T, typename... Ts>
inline constexpr std::size_t type_count_v =
    (std::size_t{0} + ... + std::size_t{std::is_same_v<T, Ts>});

}  // namespace bengal::detail

