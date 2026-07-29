#include <bengal/text/short_string.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>

namespace {

constexpr std::size_t capacity = 32;

[[noreturn]] void fail() {
  std::abort();
}

void verify(const bengal::basic_short_string<capacity>& value,
            const std::string& expected) {
  if (value.view() != std::string_view(expected) ||
      value.size() != expected.size() ||
      value.c_str()[value.size()] != '\0') {
    fail();
  }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data,
                                      std::size_t size) {
  bengal::basic_short_string<capacity> value;
  std::string expected;
  std::size_t cursor = 0;

  while (cursor < size) {
    const auto operation = data[cursor++] % 6;
    const auto requested =
        cursor < size ? static_cast<std::size_t>(data[cursor++] % 48) : 0;
    const auto available = size - cursor;
    const auto chunk_size = std::min(requested, available);
    const std::string_view chunk(
        reinterpret_cast<const char*>(data + cursor), chunk_size);
    cursor += chunk_size;

    switch (operation) {
      case 0: {
        const bool accepted = value.try_assign(chunk);
        if (accepted != (chunk.size() <= capacity)) {
          fail();
        }
        if (accepted) {
          expected.assign(chunk);
        }
        break;
      }
      case 1: {
        const auto accepted = value.assign_truncated(chunk);
        expected.assign(chunk.substr(0, capacity));
        if (accepted != expected.size()) {
          fail();
        }
        break;
      }
      case 2: {
        const bool accepted = value.try_append(chunk);
        if (accepted != (expected.size() + chunk.size() <= capacity)) {
          fail();
        }
        if (accepted) {
          expected.append(chunk);
        }
        break;
      }
      case 3:
        for (const char character : chunk) {
          const bool accepted = value.try_push_back(character);
          if (accepted != (expected.size() < capacity)) {
            fail();
          }
          if (accepted) {
            expected.push_back(character);
          }
        }
        break;
      case 4: {
        const bool removed = value.pop_back();
        if (removed != !expected.empty()) {
          fail();
        }
        if (removed) {
          expected.pop_back();
        }
        break;
      }
      case 5:
        value.clear();
        expected.clear();
        break;
    }

    verify(value, expected);
  }

  return 0;
}

