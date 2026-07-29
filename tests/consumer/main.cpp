#include <bengal/bengal.hpp>

#include <string_view>

int main() {
  bengal::basic_short_string<8> symbol(std::string_view{"BENGAL"});
  auto handlers = bengal::type_map{symbol};
  return handlers.get<bengal::basic_short_string<8>>().view() == "BENGAL"
             ? 0
             : 1;
}
