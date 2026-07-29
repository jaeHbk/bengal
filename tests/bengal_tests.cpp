#include <bengal/bengal.hpp>

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory_resource>
#include <stdexcept>
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
}

void test_type_set() {
  using numeric = bengal::type_set<int, double>;
  using mixed = bengal::type_set<double, char>;
  using combined = bengal::type_set_union_t<numeric, mixed>;
  using shared = bengal::type_set_intersection_t<numeric, mixed>;

  static_assert(numeric::size == 2);
  static_assert(numeric::contains<int>);
  static_assert(!numeric::contains<char>);
  static_assert(std::is_same_v<combined, bengal::type_set<int, double, char>>);
  static_assert(std::is_same_v<shared, bengal::type_set<double>>);

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

  bool threw = false;
  try {
    value.assign("123456");
  } catch (const std::length_error&) {
    threw = true;
  }
  CHECK(threw);
  CHECK(value.view() == "12345");
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

void test_qos_jthread() {
  std::atomic<int> result{0};

  bengal::qos_jthread plain(
      bengal::qos_class::platform_default,
      [&result](int value) { result.store(value); },
      42);
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
}

}  // namespace

int main() {
  test_type_map();
  test_type_set();
  test_short_string();
  test_static_buffer_resource();
  test_qos_jthread();

  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return 1;
  }

  std::cout << "All Bengal tests passed\n";
  return 0;
}
