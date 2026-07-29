#include <bengal/memory/static_buffer_resource.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

namespace {

[[noreturn]] void fail() {
  std::abort();
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data,
                                      std::size_t size) {
  bengal::static_buffer_resource<1024> resource;
  std::size_t cursor = 0;

  while (cursor < size) {
    const auto operation = data[cursor++] % 3;

    if (operation == 0) {
      const auto bytes =
          cursor < size ? std::size_t{1} + data[cursor++] : std::size_t{1};
      const auto alignment_power =
          cursor < size ? data[cursor++] % 8 : std::uint8_t{0};
      const auto alignment = std::size_t{1} << alignment_power;

      try {
        void* allocation = resource.allocate(bytes, alignment);
        if (reinterpret_cast<std::uintptr_t>(allocation) % alignment != 0 ||
            resource.used() > resource.capacity()) {
          fail();
        }
      } catch (const std::bad_alloc&) {
        if (resource.used() > resource.capacity()) {
          fail();
        }
      }
    } else if (operation == 1) {
      resource.release();
      if (resource.used() != 0) {
        fail();
      }
    } else {
      resource.reset_high_water_mark();
      if (resource.high_water_mark() != resource.used()) {
        fail();
      }
    }

    if (resource.high_water_mark() > resource.capacity()) {
      fail();
    }
  }

  return 0;
}

