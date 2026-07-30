#pragma once

namespace bengal {

enum class operating_system {
  linux_os,
  macos,
  windows,
  unknown,
};

enum class thread_qos_backend {
  none,
  apple,
};

struct platform_capabilities {
  operating_system system;
  thread_qos_backend thread_qos;

  constexpr bool supports_thread_qos() const noexcept {
    return thread_qos != thread_qos_backend::none;
  }
};

constexpr platform_capabilities current_platform_capabilities() noexcept {
#if defined(__APPLE__)
  return {operating_system::macos, thread_qos_backend::apple};
#elif defined(__linux__)
  return {operating_system::linux_os, thread_qos_backend::none};
#elif defined(_WIN32)
  return {operating_system::windows, thread_qos_backend::none};
#else
  return {operating_system::unknown, thread_qos_backend::none};
#endif
}

}  // namespace bengal
