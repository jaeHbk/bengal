#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <new>

namespace bengal {

template <std::size_t Capacity,
          std::size_t Alignment = alignof(std::max_align_t)>
class static_buffer_resource final : public std::pmr::memory_resource {
  static_assert(Capacity > 0,
                "bengal::static_buffer_resource requires nonzero capacity");
  static_assert(Alignment > 0 && (Alignment & (Alignment - 1)) == 0,
                "alignment must be a power of two");

 public:
  static_buffer_resource() = default;

  static_buffer_resource(const static_buffer_resource&) = delete;
  static_buffer_resource& operator=(const static_buffer_resource&) = delete;
  static_buffer_resource(static_buffer_resource&&) = delete;
  static_buffer_resource& operator=(static_buffer_resource&&) = delete;

  void release() noexcept {
    offset_ = 0;
  }

  void reset_high_water_mark() noexcept {
    high_water_mark_ = offset_;
  }

  static constexpr std::size_t capacity() noexcept {
    return Capacity;
  }

  std::size_t used() const noexcept {
    return offset_;
  }

  std::size_t remaining() const noexcept {
    return Capacity - offset_;
  }

  std::size_t high_water_mark() const noexcept {
    return high_water_mark_;
  }

 private:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    bytes = std::max<std::size_t>(bytes, 1);

    void* candidate = storage_.data() + offset_;
    auto space = Capacity - offset_;
    void* aligned = std::align(alignment, bytes, candidate, space);
    if (aligned == nullptr) {
      throw std::bad_alloc();
    }

    const auto aligned_offset =
        static_cast<std::byte*>(aligned) - storage_.data();
    offset_ = static_cast<std::size_t>(aligned_offset) + bytes;
    high_water_mark_ = std::max(high_water_mark_, offset_);
    return aligned;
  }

  void do_deallocate(void*, std::size_t, std::size_t) override {}

  bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }

  alignas(Alignment) std::array<std::byte, Capacity> storage_{};
  std::size_t offset_{0};
  std::size_t high_water_mark_{0};
};

}  // namespace bengal

