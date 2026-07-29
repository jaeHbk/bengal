#include <bengal/bengal.hpp>

#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#if __has_include(<boost/static_string/static_string.hpp>)
#include <boost/static_string/static_string.hpp>
#define BENGAL_HAS_BOOST_STATIC_STRING 1
#else
#define BENGAL_HAS_BOOST_STATIC_STRING 0
#endif

namespace {

struct quote_state {
  std::uint64_t value{1};
};

struct trade_state {
  std::uint64_t value{2};
};

constexpr std::array<std::string_view, 8> symbols{
    "AAPL", "MSFT", "NVDA", "AMZN", "META", "GOOG", "BRK.B", "BTC-USD"};

void type_map_get(benchmark::State& state) {
  bengal::type_map map{quote_state{}, trade_state{}};
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    auto& quote = map.get<quote_state>();
    quote.value += index++ & 1U;
    benchmark::DoNotOptimize(quote.value);
  }
}

void tuple_get(benchmark::State& state) {
  std::tuple<quote_state, trade_state> tuple;
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    auto& quote = std::get<quote_state>(tuple);
    quote.value += index++ & 1U;
    benchmark::DoNotOptimize(quote.value);
  }
}

void short_string_assign(benchmark::State& state) {
  bengal::basic_short_string<15> symbol;
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    benchmark::DoNotOptimize(
        symbol.try_assign(symbols[index++ % symbols.size()]));
    benchmark::DoNotOptimize(symbol.data());
  }
}

void standard_string_assign(benchmark::State& state) {
  std::string symbol;
  symbol.reserve(15);
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    symbol.assign(symbols[index++ % symbols.size()]);
    benchmark::DoNotOptimize(symbol.data());
  }
}

#if BENGAL_HAS_BOOST_STATIC_STRING
void boost_static_string_assign(benchmark::State& state) {
  boost::static_string<15> symbol;
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    const auto value = symbols[index++ % symbols.size()];
    symbol.assign(value.data(), value.size());
    benchmark::DoNotOptimize(symbol.data());
  }
}
#endif

void bengal_resource_batch(benchmark::State& state) {
  constexpr std::size_t batch_size = 64;
  bengal::static_buffer_resource<4096> resource;
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    resource.release();
    std::pmr::vector<std::uint64_t> values(&resource);
    values.reserve(batch_size);
    for (std::size_t offset = 0; offset < batch_size; ++offset) {
      values.push_back(index + offset);
    }
    ++index;
    benchmark::DoNotOptimize(values.data());
    benchmark::ClobberMemory();
  }
}

void standard_monotonic_batch(benchmark::State& state) {
  constexpr std::size_t batch_size = 64;
  alignas(std::max_align_t) std::array<std::byte, 4096> storage{};
  std::pmr::monotonic_buffer_resource resource(
      storage.data(), storage.size(), std::pmr::null_memory_resource());
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    resource.release();
    std::pmr::vector<std::uint64_t> values(&resource);
    values.reserve(batch_size);
    for (std::size_t offset = 0; offset < batch_size; ++offset) {
      values.push_back(index + offset);
    }
    ++index;
    benchmark::DoNotOptimize(values.data());
    benchmark::ClobberMemory();
  }
}

void default_allocator_batch(benchmark::State& state) {
  constexpr std::size_t batch_size = 64;
  std::size_t index = 0;

  for (auto sample : state) {
    (void)sample;
    std::vector<std::uint64_t> values;
    values.reserve(batch_size);
    for (std::size_t offset = 0; offset < batch_size; ++offset) {
      values.push_back(index + offset);
    }
    ++index;
    benchmark::DoNotOptimize(values.data());
    benchmark::ClobberMemory();
  }
}

void qos_thread_start_join(benchmark::State& state) {
  for (auto sample : state) {
    (void)sample;
    bengal::qos_jthread worker(
        bengal::qos_class::platform_default, [] {});
    benchmark::DoNotOptimize(worker.qos_status());
    worker.join();
  }
}

void standard_thread_start_join(benchmark::State& state) {
  for (auto sample : state) {
    (void)sample;
    std::thread worker([] {});
    worker.join();
  }
}

BENCHMARK(type_map_get);
BENCHMARK(tuple_get);
BENCHMARK(short_string_assign);
BENCHMARK(standard_string_assign);
#if BENGAL_HAS_BOOST_STATIC_STRING
BENCHMARK(boost_static_string_assign);
#endif
BENCHMARK(bengal_resource_batch);
BENCHMARK(standard_monotonic_batch);
BENCHMARK(default_allocator_batch);
BENCHMARK(qos_thread_start_join);
BENCHMARK(standard_thread_start_join);

}  // namespace

BENCHMARK_MAIN();

