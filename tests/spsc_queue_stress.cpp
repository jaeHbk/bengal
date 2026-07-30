#include <bengal/bengal.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>

namespace {

constexpr std::uint64_t event_count = 1'000'000;

constexpr std::uint64_t expected_checksum() noexcept {
  return event_count * (event_count - 1) / 2;
}

}  // namespace

int main() {
  bengal::spsc_queue<std::uint64_t, 1024> queue;
  std::size_t consumed = 0;
  std::uint64_t checksum = 0;
  bool ordered = true;

  const auto start = std::chrono::steady_clock::now();
  bengal::qos_jthread consumer(
      bengal::qos_class::platform_default,
      [&] {
        for (std::uint64_t expected = 0; expected < event_count;) {
          auto value = queue.try_pop();
          if (!value) {
            std::this_thread::yield();
            continue;
          }

          ordered = ordered && *value == expected;
          checksum += *value;
          ++consumed;
          ++expected;
        }
      });

  if (!consumer.startup_status().worker_started) {
    std::cerr << "consumer did not start\n";
    return 1;
  }

  for (std::uint64_t value = 0; value < event_count; ++value) {
    while (!queue.try_push(value)) {
      std::this_thread::yield();
    }
  }
  consumer.join();
  const auto finish = std::chrono::steady_clock::now();

  if (!ordered || consumed != event_count ||
      checksum != expected_checksum() || !queue.empty()) {
    std::cerr << "sustained SPSC validation failed\n";
    return 1;
  }

  const auto elapsed =
      std::chrono::duration<double>(finish - start).count();
  const auto events_per_second =
      static_cast<double>(event_count) / elapsed;
  std::cout << "events=" << event_count << " checksum=" << checksum
            << " elapsed_seconds=" << elapsed
            << " events_per_second=" << events_per_second << '\n';
  return 0;
}
