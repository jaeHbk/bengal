#pragma once

#include <bengal/detail/type_traits.hpp>

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace bengal {

template <typename... Ts>
class type_map {
  static_assert(detail::are_unique_v<Ts...>,
                "bengal::type_map requires unique value types");

 public:
  constexpr type_map()
    requires(std::is_default_constructible_v<Ts> && ...)
  = default;

  template <typename... Us>
    requires(sizeof...(Us) == sizeof...(Ts) && sizeof...(Us) > 0 &&
             (std::is_constructible_v<Ts, Us&&> && ...))
  constexpr explicit type_map(Us&&... values)
      : values_(std::forward<Us>(values)...) {}

  template <typename T>
  static constexpr bool contains = detail::contains_type_v<T, Ts...>;

  static constexpr std::size_t size = sizeof...(Ts);

  template <typename T>
    requires(contains<T>)
  constexpr T& get() & noexcept {
    return std::get<T>(values_);
  }

  template <typename T>
    requires(contains<T>)
  constexpr const T& get() const& noexcept {
    return std::get<T>(values_);
  }

  template <typename T>
    requires(contains<T>)
  constexpr T&& get() && noexcept {
    return std::get<T>(std::move(values_));
  }

  template <typename Func>
  constexpr void for_each(Func&& func) & {
    std::apply(
        [&func](auto&... values) {
          (std::invoke(func, values), ...);
        },
        values_);
  }

  template <typename Func>
  constexpr void for_each(Func&& func) const& {
    std::apply(
        [&func](const auto&... values) {
          (std::invoke(func, values), ...);
        },
        values_);
  }

 private:
  std::tuple<Ts...> values_;
};

template <typename... Us>
type_map(Us&&...) -> type_map<std::remove_cvref_t<Us>...>;

template <typename... Us>
constexpr auto make_type_map(Us&&... values) {
  return type_map<std::remove_cvref_t<Us>...>(
      std::forward<Us>(values)...);
}

template <typename T, typename... Ts>
constexpr T& get(type_map<Ts...>& map) noexcept {
  return map.template get<T>();
}

template <typename T, typename... Ts>
constexpr const T& get(const type_map<Ts...>& map) noexcept {
  return map.template get<T>();
}

template <typename T, typename... Ts>
constexpr T&& get(type_map<Ts...>&& map) noexcept {
  return std::move(map).template get<T>();
}

}  // namespace bengal

