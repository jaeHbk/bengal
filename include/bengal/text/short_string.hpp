#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstring>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string_view>

namespace bengal {

template <std::size_t Capacity>
class basic_short_string {
  static_assert(Capacity < std::numeric_limits<std::size_t>::max(),
                "basic_short_string capacity is too large");

 public:
  using value_type = char;
  using size_type = std::size_t;
  using iterator = char*;
  using const_iterator = const char*;

  constexpr basic_short_string() noexcept = default;

  explicit basic_short_string(std::string_view value) {
    assign(value);
  }

  basic_short_string& assign(std::string_view value) {
    if (!try_assign(value)) {
      throw std::length_error("bengal::basic_short_string capacity exceeded");
    }
    return *this;
  }

  bool try_assign(std::string_view value) noexcept {
    if (value.size() > Capacity) {
      return false;
    }

    if (!value.empty()) {
      std::memmove(data_.data(), value.data(), value.size());
    }
    size_ = value.size();
    data_[size_] = '\0';
    return true;
  }

  size_type assign_truncated(std::string_view value) noexcept {
    const auto accepted = std::min(value.size(), Capacity);
    if (accepted > 0) {
      std::memmove(data_.data(), value.data(), accepted);
    }
    size_ = accepted;
    data_[size_] = '\0';
    return accepted;
  }

  basic_short_string& append(std::string_view suffix) {
    if (!try_append(suffix)) {
      throw std::length_error("bengal::basic_short_string capacity exceeded");
    }
    return *this;
  }

  bool try_append(std::string_view suffix) noexcept {
    if (suffix.size() > Capacity - size_) {
      return false;
    }

    if (!suffix.empty()) {
      std::memmove(data_.data() + size_, suffix.data(), suffix.size());
    }
    size_ += suffix.size();
    data_[size_] = '\0';
    return true;
  }

  void push_back(char value) {
    if (!try_push_back(value)) {
      throw std::length_error("bengal::basic_short_string capacity exceeded");
    }
  }

  bool try_push_back(char value) noexcept {
    if (size_ == Capacity) {
      return false;
    }

    data_[size_++] = value;
    data_[size_] = '\0';
    return true;
  }

  bool pop_back() noexcept {
    if (empty()) {
      return false;
    }

    data_[--size_] = '\0';
    return true;
  }

  void clear() noexcept {
    size_ = 0;
    data_[0] = '\0';
  }

  constexpr char& operator[](size_type position) noexcept {
    return data_[position];
  }

  constexpr const char& operator[](size_type position) const noexcept {
    return data_[position];
  }

  char& at(size_type position) {
    if (position >= size_) {
      throw std::out_of_range("bengal::basic_short_string position");
    }
    return data_[position];
  }

  const char& at(size_type position) const {
    if (position >= size_) {
      throw std::out_of_range("bengal::basic_short_string position");
    }
    return data_[position];
  }

  constexpr char& front() noexcept {
    return data_[0];
  }

  constexpr const char& front() const noexcept {
    return data_[0];
  }

  constexpr char& back() noexcept {
    return data_[size_ - 1];
  }

  constexpr const char& back() const noexcept {
    return data_[size_ - 1];
  }

  constexpr char* data() noexcept {
    return data_.data();
  }

  constexpr const char* data() const noexcept {
    return data_.data();
  }

  constexpr const char* c_str() const noexcept {
    return data_.data();
  }

  constexpr iterator begin() noexcept {
    return data_.data();
  }

  constexpr const_iterator begin() const noexcept {
    return data_.data();
  }

  constexpr const_iterator cbegin() const noexcept {
    return data_.data();
  }

  constexpr iterator end() noexcept {
    return data_.data() + size_;
  }

  constexpr const_iterator end() const noexcept {
    return data_.data() + size_;
  }

  constexpr const_iterator cend() const noexcept {
    return data_.data() + size_;
  }

  constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  constexpr size_type size() const noexcept {
    return size_;
  }

  static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  constexpr std::string_view view() const noexcept {
    return std::string_view(data_.data(), size_);
  }

  constexpr explicit operator std::string_view() const noexcept {
    return view();
  }

  template <std::size_t OtherCapacity>
  constexpr bool operator==(
      const basic_short_string<OtherCapacity>& other) const noexcept {
    return view() == other.view();
  }

  template <std::size_t OtherCapacity>
  constexpr auto operator<=>(
      const basic_short_string<OtherCapacity>& other) const noexcept {
    return view() <=> other.view();
  }

 private:
  std::array<char, Capacity + 1> data_{};
  size_type size_{0};
};

template <std::size_t Capacity>
std::ostream& operator<<(std::ostream& stream,
                         const basic_short_string<Capacity>& value) {
  return stream.write(value.data(),
                      static_cast<std::streamsize>(value.size()));
}

using short_string = basic_short_string<15>;

}  // namespace bengal
