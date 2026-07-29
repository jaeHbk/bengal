#include <bengal/bengal.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using clock_type = std::chrono::steady_clock;

struct options {
  std::size_t iterations{500'000};
  std::size_t samples{15};
};

struct benchmark_result {
  std::string_view name;
  std::size_t iterations;
  std::vector<double> nanoseconds_per_operation;
  std::uint64_t checksum;
};

struct quote_state {
  std::uint64_t value{1};
};

struct trade_state {
  std::uint64_t value{2};
};

#if defined(__GNUC__) || defined(__clang__)
template <typename T>
inline void do_not_optimize(const T& value) {
  asm volatile("" : : "g"(value) : "memory");
}
#else
template <typename T>
inline void do_not_optimize(const T& value) {
  const volatile T* sink = &value;
  (void)sink;
}
#endif

std::size_t parse_size(std::string_view value, std::string_view option_name) {
  std::size_t parsed = 0;
  const auto [end, error] =
      std::from_chars(value.data(), value.data() + value.size(), parsed);
  if (error != std::errc{} || end != value.data() + value.size() ||
      parsed == 0) {
    throw std::invalid_argument(std::string(option_name) +
                                " must be a positive integer");
  }
  return parsed;
}

options parse_options(int argc, char** argv) {
  options result;

  for (int index = 1; index < argc; ++index) {
    const std::string_view argument(argv[index]);
    constexpr std::string_view iterations_prefix = "--iterations=";
    constexpr std::string_view samples_prefix = "--samples=";

    if (argument.starts_with(iterations_prefix)) {
      result.iterations =
          parse_size(argument.substr(iterations_prefix.size()), "--iterations");
    } else if (argument.starts_with(samples_prefix)) {
      result.samples =
          parse_size(argument.substr(samples_prefix.size()), "--samples");
    } else if (argument == "--help") {
      std::cout
          << "Usage: bengal_benchmarks [--iterations=N] [--samples=N]\n";
      std::exit(0);
    } else {
      throw std::invalid_argument("unknown option: " + std::string(argument));
    }
  }

  if (result.samples < 3) {
    throw std::invalid_argument("--samples must be at least 3");
  }
  return result;
}

template <typename Operation>
benchmark_result run_benchmark(std::string_view name,
                               std::size_t iterations,
                               std::size_t samples,
                               Operation&& operation) {
  const auto run_once = [&operation](std::size_t count) {
    std::uint64_t checksum = 0;
    const auto start = clock_type::now();
    for (std::size_t index = 0; index < count; ++index) {
      checksum += operation(index);
    }
    const auto finish = clock_type::now();
    do_not_optimize(checksum);

    const auto elapsed =
        std::chrono::duration<double, std::nano>(finish - start).count();
    return std::pair{elapsed / static_cast<double>(count), checksum};
  };

  const auto warmup_iterations =
      std::max<std::size_t>(1, iterations / 10);
  (void)run_once(warmup_iterations);

  benchmark_result result{name, iterations, {}, 0};
  result.nanoseconds_per_operation.reserve(samples);
  for (std::size_t sample = 0; sample < samples; ++sample) {
    const auto [nanoseconds, checksum] = run_once(iterations);
    result.nanoseconds_per_operation.push_back(nanoseconds);
    result.checksum ^= checksum;
  }
  std::sort(result.nanoseconds_per_operation.begin(),
            result.nanoseconds_per_operation.end());
  return result;
}

double median(const std::vector<double>& sorted_values) {
  const auto middle = sorted_values.size() / 2;
  if (sorted_values.size() % 2 == 0) {
    return (sorted_values[middle - 1] + sorted_values[middle]) / 2.0;
  }
  return sorted_values[middle];
}

double percentile_95(const std::vector<double>& sorted_values) {
  const auto rank = (95 * sorted_values.size() + 99) / 100;
  return sorted_values[std::max<std::size_t>(1, rank) - 1];
}

void print_results(const std::vector<benchmark_result>& results,
                   const options& configuration) {
#if defined(NDEBUG)
  constexpr std::string_view build_mode = "release";
#else
  constexpr std::string_view build_mode = "debug";
#endif

  std::cout << "Bengal benchmark harness\n"
            << "compiler=" << __VERSION__ << '\n'
            << "cxx=" << __cplusplus << '\n'
            << "build_mode=" << build_mode << '\n'
            << "hardware_threads=" << std::thread::hardware_concurrency()
            << '\n'
            << "base_iterations=" << configuration.iterations << '\n'
            << "samples=" << configuration.samples << "\n\n"
            << std::left << std::setw(36) << "benchmark" << std::right
            << std::setw(14) << "median ns/op" << std::setw(14) << "p95 ns/op"
            << std::setw(14) << "min ns/op" << std::setw(14) << "iterations"
            << '\n';

  for (const auto& result : results) {
    std::cout << std::left << std::setw(36) << result.name << std::right
              << std::fixed << std::setprecision(2) << std::setw(14)
              << median(result.nanoseconds_per_operation) << std::setw(14)
              << percentile_95(result.nanoseconds_per_operation)
              << std::setw(14) << result.nanoseconds_per_operation.front()
              << std::setw(14) << result.iterations << '\n';
    do_not_optimize(result.checksum);
  }
}

std::vector<benchmark_result> run_suite(const options& configuration) {
  std::vector<benchmark_result> results;
  results.reserve(7);

  bengal::type_map map{quote_state{}, trade_state{}};
  results.push_back(run_benchmark(
      "type_map/get",
      configuration.iterations,
      configuration.samples,
      [&map](std::size_t index) {
        auto& quote = map.get<quote_state>();
        quote.value += index & 1U;
        do_not_optimize(&quote);
        return quote.value;
      }));

  std::tuple<quote_state, trade_state> tuple;
  results.push_back(run_benchmark(
      "std::tuple/get",
      configuration.iterations,
      configuration.samples,
      [&tuple](std::size_t index) {
        auto& quote = std::get<quote_state>(tuple);
        quote.value += index & 1U;
        do_not_optimize(&quote);
        return quote.value;
      }));

  constexpr std::array<std::string_view, 8> symbols{
      "AAPL", "MSFT", "NVDA", "AMZN", "META", "GOOG", "BRK.B", "BTC-USD"};

  bengal::basic_short_string<15> short_symbol;
  results.push_back(run_benchmark(
      "basic_short_string/assign",
      configuration.iterations,
      configuration.samples,
      [&short_symbol, &symbols](std::size_t index) {
        const bool assigned =
            short_symbol.try_assign(symbols[index % symbols.size()]);
        do_not_optimize(short_symbol.data());
        return static_cast<std::uint64_t>(assigned) + short_symbol.size() +
               static_cast<unsigned char>(short_symbol.front());
      }));

  std::string standard_symbol;
  standard_symbol.reserve(15);
  results.push_back(run_benchmark(
      "std::string/assign",
      configuration.iterations,
      configuration.samples,
      [&standard_symbol, &symbols](std::size_t index) {
        standard_symbol.assign(symbols[index % symbols.size()]);
        do_not_optimize(standard_symbol.data());
        return standard_symbol.size() +
               static_cast<unsigned char>(standard_symbol.front());
      }));

  const auto batch_iterations =
      std::max<std::size_t>(1'000, configuration.iterations / 64);
  constexpr std::size_t batch_size = 64;

  bengal::static_buffer_resource<4096> bengal_resource;
  results.push_back(run_benchmark(
      "pmr_vector/bengal_resource",
      batch_iterations,
      configuration.samples,
      [&bengal_resource](std::size_t index) {
        bengal_resource.release();
        std::pmr::vector<std::uint64_t> values(&bengal_resource);
        values.reserve(batch_size);
        for (std::size_t offset = 0; offset < batch_size; ++offset) {
          values.push_back(index + offset);
        }
        do_not_optimize(values.data());
        return values.back();
      }));

  alignas(std::max_align_t) std::array<std::byte, 4096> standard_storage{};
  std::pmr::monotonic_buffer_resource standard_resource(
      standard_storage.data(),
      standard_storage.size(),
      std::pmr::null_memory_resource());
  results.push_back(run_benchmark(
      "pmr_vector/std_monotonic_resource",
      batch_iterations,
      configuration.samples,
      [&standard_resource](std::size_t index) {
        standard_resource.release();
        std::pmr::vector<std::uint64_t> values(&standard_resource);
        values.reserve(batch_size);
        for (std::size_t offset = 0; offset < batch_size; ++offset) {
          values.push_back(index + offset);
        }
        do_not_optimize(values.data());
        return values.back();
      }));

  results.push_back(run_benchmark(
      "vector/default_allocator",
      batch_iterations,
      configuration.samples,
      [](std::size_t index) {
        std::vector<std::uint64_t> values;
        values.reserve(batch_size);
        for (std::size_t offset = 0; offset < batch_size; ++offset) {
          values.push_back(index + offset);
        }
        do_not_optimize(values.data());
        return values.back();
      }));

  return results;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const auto configuration = parse_options(argc, argv);
    const auto results = run_suite(configuration);
    print_results(results, configuration);
  } catch (const std::exception& error) {
    std::cerr << "benchmark error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
