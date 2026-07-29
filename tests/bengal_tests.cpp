#include <bengal/bengal.hpp>

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

int failures = 0;

#define CHECK(condition)                                                     \
  do {                                                                       \
    if (!(condition)) {                                                      \
      std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: "        \
                << #condition << '\n';                                       \
      ++failures;                                                            \
    }                                                                        \
  } while (false)

struct price_handler {
  int calls{0};
};

struct quote_handler {
  int calls{0};
};

void test_type_map() {
  auto handlers = bengal::type_map{price_handler{}, quote_handler{}};

  static_assert(decltype(handlers)::size == 2);
  static_assert(decltype(handlers)::contains<price_handler>);
  static_assert(!decltype(handlers)::contains<int>);

  bengal::get<price_handler>(handlers).calls = 3;
  CHECK(handlers.get<price_handler>().calls == 3);

  int visits = 0;
  const auto& const_handlers = handlers;
  const_handlers.for_each([&visits](const auto&) { ++visits; });
  CHECK(visits == 2);

  auto move_only = bengal::make_type_map(std::make_unique<int>(7));
  CHECK(*move_only.get<std::unique_ptr<int>>() == 7);

  price_handler original{5};
  auto decayed = bengal::type_map{original};
  static_assert(std::is_same_v<decltype(decayed),
                               bengal::type_map<price_handler>>);
  decayed.get<price_handler>().calls = 8;
  CHECK(original.calls == 5);
}

void test_type_set() {
  using numeric = bengal::type_set<int, double>;
  using mixed = bengal::type_set<double, char>;
  using combined = bengal::type_set_union_t<numeric, mixed>;
  using shared = bengal::type_set_intersection_t<numeric, mixed>;
  using duplicate_union =
      bengal::type_set_union_t<numeric, bengal::type_set<int, double>>;

  static_assert(numeric::size == 2);
  static_assert(numeric::contains<int>);
  static_assert(!numeric::contains<char>);
  static_assert(std::is_same_v<combined, bengal::type_set<int, double, char>>);
  static_assert(std::is_same_v<shared, bengal::type_set<double>>);
  static_assert(std::is_same_v<duplicate_union, numeric>);

  int visits = 0;
  bengal::for_each_type(combined{}, [&visits](auto) { ++visits; });
  CHECK(visits == 3);
}

void test_short_string() {
  bengal::basic_short_string<5> empty;
  CHECK(empty.try_assign({}));
  CHECK(empty.empty());
  CHECK(empty.c_str()[0] == '\0');

  bengal::basic_short_string<5> value(std::string_view{"abc"});
  CHECK(value.size() == 3);
  CHECK(value.view() == "abc");
  CHECK(value.c_str()[3] == '\0');

  CHECK(value.try_append("de"));
  CHECK(value.view() == "abcde");
  CHECK(!value.try_push_back('f'));
  CHECK(!value.try_assign("longer"));
  CHECK(value.view() == "abcde");

  value.clear();
  CHECK(value.empty());
  CHECK(value.try_assign("a"));

  bengal::basic_short_string<5> other(std::string_view{"a"});
  CHECK(value == other);

  CHECK(value.assign_truncated("1234567") == 5);
  CHECK(value.view() == "12345");

  const std::string_view overlapping(value.data() + 1, 3);
  CHECK(value.try_assign(overlapping));
  CHECK(value.view() == "234");

  bengal::basic_short_string<8> wider(std::string_view{"234"});
  CHECK(value == wider);

  bool threw = false;
  try {
    value.assign("123456");
  } catch (const std::length_error&) {
    threw = true;
  }
  CHECK(threw);
  CHECK(value.view() == "234");
}

void test_short_string_properties() {
  constexpr std::size_t capacity = 15;
  bengal::basic_short_string<capacity> value;
  std::string expected;
  std::mt19937_64 random(0xBEE6A1ULL);

  const auto random_text = [&random](std::size_t size) {
    std::string result(size, '\0');
    for (char& character : result) {
      character = static_cast<char>('a' + random() % 26);
    }
    return result;
  };

  for (std::size_t iteration = 0; iteration < 10'000; ++iteration) {
    switch (random() % 6) {
      case 0: {
        const auto candidate = random_text(random() % 21);
        const bool accepted = value.try_assign(candidate);
        CHECK(accepted == (candidate.size() <= capacity));
        if (accepted) {
          expected = candidate;
        }
        break;
      }
      case 1: {
        const auto suffix = random_text(random() % 8);
        const bool accepted = value.try_append(suffix);
        CHECK(accepted == (expected.size() + suffix.size() <= capacity));
        if (accepted) {
          expected += suffix;
        }
        break;
      }
      case 2: {
        const char character = static_cast<char>('a' + random() % 26);
        const bool accepted = value.try_push_back(character);
        CHECK(accepted == (expected.size() < capacity));
        if (accepted) {
          expected.push_back(character);
        }
        break;
      }
      case 3: {
        const bool removed = value.pop_back();
        CHECK(removed == !expected.empty());
        if (removed) {
          expected.pop_back();
        }
        break;
      }
      case 4:
        value.clear();
        expected.clear();
        break;
      case 5: {
        const auto candidate = random_text(random() % 21);
        const auto accepted = value.assign_truncated(candidate);
        expected = candidate.substr(0, capacity);
        CHECK(accepted == expected.size());
        break;
      }
    }

    CHECK(value.view() == expected);
    CHECK(value.size() == expected.size());
    CHECK(value.c_str()[value.size()] == '\0');
  }
}

void test_static_buffer_resource() {
  bengal::static_buffer_resource<256> resource;

  {
    std::pmr::vector<int> values(&resource);
    for (int value = 0; value < 16; ++value) {
      values.push_back(value);
    }
    CHECK(values.size() == 16);
    CHECK(resource.used() > 0);
  }

  const auto high_water = resource.high_water_mark();
  CHECK(high_water > 0);
  resource.release();
  CHECK(resource.used() == 0);
  CHECK(resource.high_water_mark() == high_water);
  resource.reset_high_water_mark();
  CHECK(resource.high_water_mark() == 0);

  void* aligned = resource.allocate(1, 64);
  CHECK(reinterpret_cast<std::uintptr_t>(aligned) % 64 == 0);
  resource.release();

  bool exhausted = false;
  try {
    (void)resource.allocate(512);
  } catch (const std::bad_alloc&) {
    exhausted = true;
  }
  CHECK(exhausted);
}

void test_static_buffer_resource_properties() {
  bengal::static_buffer_resource<4096> resource;
  std::mt19937_64 random(0xA110CA7EULL);

  for (std::size_t cycle = 0; cycle < 100; ++cycle) {
    while (true) {
      const auto bytes = std::size_t{1} + random() % 64;
      const auto alignment = std::size_t{1} << (random() % 7);
      try {
        void* allocation = resource.allocate(bytes, alignment);
        CHECK(reinterpret_cast<std::uintptr_t>(allocation) % alignment == 0);
        CHECK(resource.used() <= resource.capacity());
      } catch (const std::bad_alloc&) {
        break;
      }
    }

    CHECK(resource.high_water_mark() <= resource.capacity());
    resource.release();
    CHECK(resource.used() == 0);
  }
}

void test_qos_jthread() {
  std::atomic<int> result{0};

  bengal::qos_jthread plain(
      bengal::qos_class::platform_default,
      [&result](int value) { result.store(value); },
      42);
  CHECK(!plain.qos_status());
  plain.join();
  CHECK(result.load() == 42);

  std::atomic<bool> observed_stop{false};
  bengal::qos_jthread stoppable(
      bengal::qos_class::platform_default,
      [&observed_stop](std::stop_token token) {
        while (!token.stop_requested()) {
          std::this_thread::yield();
        }
        observed_stop.store(true);
      });
  CHECK(stoppable.request_stop());
  stoppable.join();
  CHECK(observed_stop.load());

  if constexpr (!bengal::qos_available) {
    bengal::qos_jthread unsupported(
        bengal::qos_class::background, [] {});
    CHECK(unsupported.qos_status() ==
          std::make_error_code(std::errc::operation_not_supported));
    unsupported.join();
  }
}

}  // namespace

int main() {
  test_type_map();
  test_type_set();
  test_short_string();
  test_short_string_properties();
  test_static_buffer_resource();
  test_static_buffer_resource_properties();
  test_qos_jthread();

  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return 1;
  }

  std::cout << "All Bengal tests passed\n";
  return 0;
}
