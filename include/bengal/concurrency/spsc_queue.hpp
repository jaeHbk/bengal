#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace bengal {

template <typename T, std::size_t Capacity>
class spsc_queue {
  static_assert(Capacity > 0, "spsc_queue capacity must be positive");
  static_assert(Capacity < std::numeric_limits<std::size_t>::max(),
                "spsc_queue capacity is too large");

 public:
  using value_type = T;
  using size_type = std::size_t;

  static constexpr size_type capacity = Capacity;
  static constexpr bool indices_are_always_lock_free =
      std::atomic<size_type>::is_always_lock_free;

  spsc_queue() noexcept = default;
  spsc_queue(const spsc_queue&) = delete;
  spsc_queue& operator=(const spsc_queue&) = delete;
  spsc_queue(spsc_queue&&) = delete;
  spsc_queue& operator=(spsc_queue&&) = delete;
  ~spsc_queue() = default;

  template <typename... Args>
    requires std::is_constructible_v<T, Args...>
  bool try_emplace(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>) {
    const auto write = write_index_.value.load(std::memory_order_relaxed);
    const auto next = increment(write);
    if (next == read_index_.value.load(std::memory_order_acquire)) {
      return false;
    }

    slots_[write].emplace(std::forward<Args>(args)...);
    write_index_.value.store(next, std::memory_order_release);
    return true;
  }

  bool try_push(const T& value) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    return try_emplace(value);
  }

  bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
    return try_emplace(std::move(value));
  }

  std::optional<T> try_pop() noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_destructible_v<T>) {
    const auto read = read_index_.value.load(std::memory_order_relaxed);
    if (read == write_index_.value.load(std::memory_order_acquire)) {
      return std::nullopt;
    }

    std::optional<T> result(std::in_place, std::move(*slots_[read]));
    slots_[read].reset();
    read_index_.value.store(increment(read), std::memory_order_release);
    return result;
  }

  template <typename Func>
    requires std::is_invocable_v<Func, T&>
  bool consume_one(Func&& consumer) noexcept(
      std::is_nothrow_invocable_v<Func, T&> &&
      std::is_nothrow_destructible_v<T>) {
    const auto read = read_index_.value.load(std::memory_order_relaxed);
    if (read == write_index_.value.load(std::memory_order_acquire)) {
      return false;
    }

    std::invoke(std::forward<Func>(consumer), *slots_[read]);
    slots_[read].reset();
    read_index_.value.store(increment(read), std::memory_order_release);
    return true;
  }

  bool empty() const noexcept {
    return read_index_.value.load(std::memory_order_relaxed) ==
           write_index_.value.load(std::memory_order_acquire);
  }

  bool full() const noexcept {
    const auto write = write_index_.value.load(std::memory_order_relaxed);
    return increment(write) ==
           read_index_.value.load(std::memory_order_acquire);
  }

  size_type size_approx() const noexcept {
    const auto read = read_index_.value.load(std::memory_order_acquire);
    const auto write = write_index_.value.load(std::memory_order_acquire);
    return write >= read ? write - read : slot_count - read + write;
  }

 private:
  static constexpr size_type slot_count = Capacity + 1;
  static constexpr std::size_t cache_line_alignment = 64;

  struct alignas(cache_line_alignment) aligned_index {
    std::atomic<size_type> value{0};
  };

  static constexpr size_type increment(size_type index) noexcept {
    ++index;
    return index == slot_count ? 0 : index;
  }

  std::array<std::optional<T>, slot_count> slots_{};
  aligned_index write_index_{};
  aligned_index read_index_{};
};

}  // namespace bengal
