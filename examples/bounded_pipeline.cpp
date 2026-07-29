#include <bengal/bengal.hpp>

#include <cstdint>
#include <iostream>
#include <memory_resource>
#include <string_view>
#include <vector>

namespace {

struct market_event {
  bengal::basic_short_string<15> symbol;
  std::int64_t price_micros;
};

struct event_handler {
  std::size_t processed{0};
  std::int64_t last_price_micros{0};

  void handle(const market_event& event) {
    ++processed;
    last_price_micros = event.price_micros;
  }
};

}  // namespace

int main() {
  bengal::static_buffer_resource<4096> memory;
  std::pmr::vector<market_event> batch(&memory);
  batch.reserve(3);
  batch.push_back(
      {bengal::basic_short_string<15>(std::string_view{"AAPL"}), 21'525'000});
  batch.push_back(
      {bengal::basic_short_string<15>(std::string_view{"MSFT"}), 51'240'000});
  batch.push_back(
      {bengal::basic_short_string<15>(std::string_view{"NVDA"}), 17'575'000});

  auto handlers = bengal::type_map{event_handler{}};
  bengal::qos_jthread worker(
      bengal::qos_class::platform_default,
      [&batch, &handlers] {
        for (const auto& event : batch) {
          handlers.get<event_handler>().handle(event);
        }
      });
  worker.join();

  const auto& handler = handlers.get<event_handler>();
  std::cout << "processed=" << handler.processed
            << " last_price_micros=" << handler.last_price_micros
            << " memory_high_water=" << memory.high_water_mark() << '\n';
}

