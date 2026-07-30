#include <bengal/bengal.hpp>

#include <string_view>

int main() {
  static_assert(BENGAL_VERSION_MAJOR == bengal::version_major);
  static_assert(BENGAL_VERSION_MINOR == bengal::version_minor);
  static_assert(BENGAL_VERSION_PATCH == bengal::version_patch);
  static_assert(bengal::version == "0.3.0");

  bengal::basic_short_string<8> symbol(std::string_view{"BENGAL"});
  auto handlers = bengal::type_map{symbol};

  bengal::spsc_queue<bengal::basic_short_string<8>, 1> queue;
  if (!queue.try_push(symbol)) {
    return 1;
  }
  const auto received = queue.try_pop();
  const auto& installed_symbol =
      handlers.get<bengal::basic_short_string<8>>();

  constexpr auto capabilities = bengal::current_platform_capabilities();
  static_assert(capabilities.supports_thread_qos() == bengal::qos_available);

  return received && received->view() == "BENGAL" &&
                 installed_symbol.view() == "BENGAL"
             ? 0 : 1;
}
