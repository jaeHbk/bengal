#pragma once

#include <bengal/platform/capabilities.hpp>

#include <atomic>
#include <cerrno>
#include <functional>
#include <future>
#include <memory>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(__APPLE__)
#include <pthread.h>
#include <sys/qos.h>
#endif

namespace bengal {

namespace detail {

struct stop_state {
  std::atomic<bool> requested{false};
};

}  // namespace detail

class stop_token {
 public:
  stop_token() noexcept = default;

  bool stop_requested() const noexcept {
    return state_ != nullptr &&
           state_->requested.load(std::memory_order_acquire);
  }

  bool stop_possible() const noexcept {
    return state_ != nullptr;
  }

 private:
  explicit stop_token(std::shared_ptr<detail::stop_state> state) noexcept
      : state_(std::move(state)) {}

  std::shared_ptr<detail::stop_state> state_;

  friend class stop_source;
};

class stop_source {
 public:
  stop_source() : state_(std::make_shared<detail::stop_state>()) {}

  stop_token get_token() const noexcept {
    return stop_token(state_);
  }

  bool stop_requested() const noexcept {
    return state_ != nullptr &&
           state_->requested.load(std::memory_order_acquire);
  }

  bool request_stop() noexcept {
    return state_ != nullptr &&
           !state_->requested.exchange(true, std::memory_order_acq_rel);
  }

 private:
  std::shared_ptr<detail::stop_state> state_;
};

enum class qos_class {
  platform_default,
  user_interactive,
  user_initiated,
  utility,
  background,
};

inline constexpr bool qos_available =
    current_platform_capabilities().supports_thread_qos();

enum class qos_outcome {
  not_requested,
  applied,
  unsupported,
  failed,
};

struct thread_startup_status {
  bool worker_started{false};
  qos_class requested_qos{qos_class::platform_default};
  qos_outcome qos{qos_outcome::not_requested};
  std::error_code error{};

  bool qos_applied() const noexcept {
    return qos == qos_outcome::applied;
  }
};

inline std::error_code set_current_thread_qos(qos_class value) noexcept {
  if (value == qos_class::platform_default) {
    return {};
  }

#if defined(__APPLE__)
  qos_class_t native_class = QOS_CLASS_DEFAULT;
  switch (value) {
    case qos_class::user_interactive:
      native_class = QOS_CLASS_USER_INTERACTIVE;
      break;
    case qos_class::user_initiated:
      native_class = QOS_CLASS_USER_INITIATED;
      break;
    case qos_class::utility:
      native_class = QOS_CLASS_UTILITY;
      break;
    case qos_class::background:
      native_class = QOS_CLASS_BACKGROUND;
      break;
    case qos_class::platform_default:
      return {};
  }

  const int result = pthread_set_qos_class_self_np(native_class, 0);
  return result == 0 ? std::error_code{}
                     : std::error_code(result, std::generic_category());
#else
  return std::make_error_code(std::errc::operation_not_supported);
#endif
}

namespace detail {

inline thread_startup_status configure_current_thread(
    qos_class value) noexcept {
  thread_startup_status status{
      true, value, qos_outcome::not_requested, {}};
  if (value == qos_class::platform_default) {
    return status;
  }

  status.error = set_current_thread_qos(value);
  if (!status.error) {
    status.qos = qos_outcome::applied;
  } else if (status.error ==
             std::make_error_code(std::errc::operation_not_supported)) {
    status.qos = qos_outcome::unsupported;
  } else {
    status.qos = qos_outcome::failed;
  }
  return status;
}

}  // namespace detail

class qos_jthread {
 public:
  using id = std::thread::id;
  using native_handle_type = std::thread::native_handle_type;

  qos_jthread() = default;

  template <typename Func, typename... Args>
  explicit qos_jthread(qos_class qos, Func&& func, Args&&... args)
      : thread_() {
    std::promise<thread_startup_status> startup_promise;
    auto startup_future = startup_promise.get_future();
    auto token = stop_source_.get_token();

    thread_ = std::thread(
        [qos,
         startup_promise = std::move(startup_promise),
         token = std::move(token),
         callable = std::forward<Func>(func),
         arguments =
             std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...)]()
            mutable {
          startup_promise.set_value(detail::configure_current_thread(qos));

          std::apply(
              [&callable, &token](auto&&... inner_args) {
                if constexpr (std::is_invocable_v<
                                  decltype(callable)&,
                                  stop_token,
                                  decltype(inner_args)...>) {
                  std::invoke(
                      callable,
                      token,
                      std::forward<decltype(inner_args)>(inner_args)...);
                } else {
                  static_assert(
                      std::is_invocable_v<decltype(callable)&,
                                          decltype(inner_args)...>,
                      "qos_jthread callable cannot be invoked with its "
                      "arguments");
                  std::invoke(
                      callable,
                      std::forward<decltype(inner_args)>(inner_args)...);
                }
              },
              std::move(arguments));
        });

    startup_status_ = startup_future.get();
  }

  qos_jthread(const qos_jthread&) = delete;
  qos_jthread& operator=(const qos_jthread&) = delete;
  qos_jthread(qos_jthread&&) noexcept = default;

  qos_jthread& operator=(qos_jthread&& other) noexcept {
    if (this != &other) {
      stop_and_join();
      thread_ = std::move(other.thread_);
      stop_source_ = std::move(other.stop_source_);
      startup_status_ = other.startup_status_;
    }
    return *this;
  }

  ~qos_jthread() {
    stop_and_join();
  }

  bool joinable() const noexcept {
    return thread_.joinable();
  }

  id get_id() const noexcept {
    return thread_.get_id();
  }

  native_handle_type native_handle() {
    return thread_.native_handle();
  }

  stop_source get_stop_source() const noexcept {
    return stop_source_;
  }

  stop_token get_stop_token() const noexcept {
    return stop_source_.get_token();
  }

  bool request_stop() noexcept {
    return stop_source_.request_stop();
  }

  const std::error_code& qos_status() const noexcept {
    return startup_status_.error;
  }

  const thread_startup_status& startup_status() const noexcept {
    return startup_status_;
  }

  void join() {
    thread_.join();
  }

  void detach() {
    thread_.detach();
  }

  void swap(qos_jthread& other) noexcept {
    thread_.swap(other.thread_);
    using std::swap;
    swap(stop_source_, other.stop_source_);
    swap(startup_status_, other.startup_status_);
  }

 private:
  void stop_and_join() noexcept {
    if (thread_.joinable()) {
      request_stop();
      thread_.join();
    }
  }

  std::thread thread_;
  stop_source stop_source_;
  thread_startup_status startup_status_;
};

}  // namespace bengal
