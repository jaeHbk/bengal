#include <bengal/bengal.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <thread>
#include <utility>

namespace {

enum class event_kind {
  trade,
  end,
};

struct market_event {
  event_kind kind;
  bengal::basic_short_string<15> symbol;
  std::int64_t price_micros;
};

struct event_handler {
  std::size_t processed{0};
  std::int64_t last_price_micros{0};

  void handle(const market_event& event) noexcept {
    ++processed;
    last_price_micros = event.price_micros;
  }
};

}  // namespace

int main() {
  bengal::spsc_queue<market_event, 2> queue;
  auto handlers = bengal::type_map{event_handler{}};

  bengal::qos_jthread worker(
      bengal::qos_class::platform_default,
      [&queue, &handlers] {
        bool finished = false;
        while (!finished) {
          if (!queue.consume_one([&](market_event& event) noexcept {
                if (event.kind == event_kind::end) {
                  finished = true;
                } else {
                  handlers.get<event_handler>().handle(event);
                }
              })) {
            std::this_thread::yield();
          }
        }
      });

  if (!worker.startup_status().worker_started) {
    std::cerr << "worker startup failed\n";
    return 1;
  }

  std::size_t backpressure_events = 0;
  const auto publish = [&queue, &backpressure_events](market_event event) {
    while (!queue.try_push(std::move(event))) {
      ++backpressure_events;
      std::this_thread::yield();
    }
  };

  publish({event_kind::trade,
           bengal::basic_short_string<15>(std::string_view{"AAPL"}),
           21'525'000});
  publish({event_kind::trade,
           bengal::basic_short_string<15>(std::string_view{"MSFT"}),
           51'240'000});
  publish({event_kind::trade,
           bengal::basic_short_string<15>(std::string_view{"NVDA"}),
           17'575'000});
  publish({event_kind::end, {}, 0});
  worker.join();

  const auto& handler = handlers.get<event_handler>();
  std::cout << "processed=" << handler.processed
            << " last_price_micros=" << handler.last_price_micros
            << " backpressure_events=" << backpressure_events
            << " lock_free_indices="
            << decltype(queue)::indices_are_always_lock_free << '\n';
  return handler.processed == 3 ? 0 : 1;
}
