#include <bengal/bengal.hpp>

#include <concepts>
#include <cstddef>
#include <memory_resource>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

namespace {

template <typename Queue>
concept queue_contract =
    requires(Queue& queue, typename Queue::value_type value) {
      typename Queue::size_type;
      { Queue::capacity } -> std::convertible_to<std::size_t>;
      { queue.try_push(std::move(value)) } -> std::same_as<bool>;
      { queue.try_pop() };
      { queue.empty() } -> std::same_as<bool>;
      { queue.full() } -> std::same_as<bool>;
      { queue.size_approx() } -> std::same_as<typename Queue::size_type>;
    };

static_assert(BENGAL_VERSION_MAJOR == 1);
static_assert(BENGAL_VERSION_MINOR == 0);
static_assert(BENGAL_VERSION_PATCH == 0);
static_assert(bengal::version == std::string_view{"1.0.0"});

static_assert(queue_contract<bengal::spsc_queue<int, 8>>);
static_assert(!std::copyable<bengal::spsc_queue<int, 8>>);
static_assert(std::derived_from<bengal::static_buffer_resource<64>,
                                std::pmr::memory_resource>);
static_assert(std::same_as<bengal::short_string,
                           bengal::basic_short_string<15>>);
static_assert(bengal::type_set<int, double>::contains<int>);
static_assert(bengal::type_map<int, double>::contains<double>);
static_assert(std::same_as<bengal::qos_jthread::id, std::thread::id>);

}  // namespace

int main() {
  return 0;
}
