#pragma once

#include <bengal/detail/type_traits.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace bengal {

template <typename... Ts>
struct type_set {
  static_assert(detail::are_unique_v<Ts...>,
                "bengal::type_set requires unique types");

  static constexpr std::size_t size = sizeof...(Ts);

  template <typename T>
  static constexpr bool contains = detail::contains_type_v<T, Ts...>;
};

namespace detail {

template <typename Set, typename T, bool Present>
struct type_set_insert_choice;

template <typename Set, typename T>
struct type_set_insert_choice<Set, T, true> {
  using type = Set;
};

template <typename... Ts, typename T>
struct type_set_insert_choice<type_set<Ts...>, T, false> {
  using type = type_set<Ts..., T>;
};

template <typename Set, typename T>
struct type_set_insert;

template <typename... Ts, typename T>
struct type_set_insert<type_set<Ts...>, T>
    : type_set_insert_choice<type_set<Ts...>,
                             T,
                             contains_type_v<T, Ts...>> {};

template <typename Set, typename... Ts>
struct type_set_insert_many;

template <typename Set>
struct type_set_insert_many<Set> {
  using type = Set;
};

template <typename Set, typename T, typename... Ts>
struct type_set_insert_many<Set, T, Ts...> {
  using inserted = typename type_set_insert<Set, T>::type;
  using type = typename type_set_insert_many<inserted, Ts...>::type;
};

template <typename Result, typename Right, typename... Ts>
struct type_set_intersection_impl;

template <typename Result, typename Right>
struct type_set_intersection_impl<Result, Right> {
  using type = Result;
};

template <typename Result, typename... Us, typename T, typename... Ts>
struct type_set_intersection_impl<Result, type_set<Us...>, T, Ts...> {
  using next = typename type_set_insert_choice<
      Result,
      T,
      !contains_type_v<T, Us...>>::type;
  using type =
      typename type_set_intersection_impl<next, type_set<Us...>, Ts...>::type;
};

}  // namespace detail

template <typename Left, typename Right>
struct type_set_union;

template <typename... Ts, typename... Us>
struct type_set_union<type_set<Ts...>, type_set<Us...>> {
  using type =
      typename detail::type_set_insert_many<type_set<Ts...>, Us...>::type;
};

template <typename Left, typename Right>
using type_set_union_t = typename type_set_union<Left, Right>::type;

template <typename Left, typename Right>
struct type_set_intersection;

template <typename... Ts, typename... Us>
struct type_set_intersection<type_set<Ts...>, type_set<Us...>> {
  using type = typename detail::type_set_intersection_impl<
      type_set<>,
      type_set<Us...>,
      Ts...>::type;
};

template <typename Left, typename Right>
using type_set_intersection_t =
    typename type_set_intersection<Left, Right>::type;

template <typename... Ts, typename Func>
constexpr void for_each_type(type_set<Ts...>, Func&& func) {
  (std::invoke(func, std::type_identity<Ts>{}), ...);
}

}  // namespace bengal

