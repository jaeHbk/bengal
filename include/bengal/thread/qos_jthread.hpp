#pragma once

#include <cerrno>
#include <functional>
#include <future>
#include <stop_token>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>

#if !defined(__cpp_lib_jthread) || __cpp_lib_jthread < 201911L
#error "bengal::qos_jthread requires std::jthread and std::stop_token support"
#endif

#if defined(__APPLE__)
#include <pthread.h>
#include <sys/qos.h>
#endif

namespace bengal {

enum class qos_class {
  platform_default,
  user_interactive,
  user_initiated,
  utility,
  background,
};

#if defined(__APPLE__)
inline constexpr bool qos_available = true;
#else
inline constexpr bool qos_available = false;
#endif

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

class qos_jthread {
 public:
  using id = std::jthread::id;
  using native_handle_type = std::jthread::native_handle_type;

  qos_jthread() noexcept = default;

  template <typename Func, typename... Args>
  explicit qos_jthread(qos_class qos, Func&& func, Args&&... args)
      : thread_() {
    std::promise<std::error_code> startup_promise;
    auto startup_future = startup_promise.get_future();

    thread_ = std::jthread(
        [qos,
         startup_promise = std::move(startup_promise),
         callable = std::forward<Func>(func)](
            std::stop_token token, auto&&... inner_args) mutable {
          startup_promise.set_value(set_current_thread_qos(qos));

          if constexpr (std::is_invocable_v<
                            decltype(callable)&,
                            std::stop_token,
                            decltype(inner_args)...>) {
            std::invoke(callable,
                        token,
                        std::forward<decltype(inner_args)>(inner_args)...);
          } else {
            static_assert(
                std::is_invocable_v<decltype(callable)&,
                                    decltype(inner_args)...>,
                "qos_jthread callable cannot be invoked with its arguments");
            std::invoke(
                callable,
                std::forward<decltype(inner_args)>(inner_args)...);
          }
        },
        std::forward<Args>(args)...);

    qos_status_ = startup_future.get();
  }

  qos_jthread(const qos_jthread&) = delete;
  qos_jthread& operator=(const qos_jthread&) = delete;
  qos_jthread(qos_jthread&&) noexcept = default;
  qos_jthread& operator=(qos_jthread&&) noexcept = default;

  bool joinable() const noexcept {
    return thread_.joinable();
  }

  id get_id() const noexcept {
    return thread_.get_id();
  }

  native_handle_type native_handle() {
    return thread_.native_handle();
  }

  std::stop_source get_stop_source() noexcept {
    return thread_.get_stop_source();
  }

  std::stop_token get_stop_token() const noexcept {
    return thread_.get_stop_token();
  }

  bool request_stop() noexcept {
    return thread_.request_stop();
  }

  const std::error_code& qos_status() const noexcept {
    return qos_status_;
  }

  void join() {
    thread_.join();
  }

  void detach() {
    thread_.detach();
  }

  void swap(qos_jthread& other) noexcept {
    thread_.swap(other.thread_);
  }

 private:
  std::jthread thread_;
  std::error_code qos_status_;
};

}  // namespace bengal
