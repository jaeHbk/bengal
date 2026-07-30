#pragma once

#include <string_view>

#define BENGAL_VERSION_MAJOR 0
#define BENGAL_VERSION_MINOR 3
#define BENGAL_VERSION_PATCH 0
#define BENGAL_VERSION_STRING "0.3.0"

namespace bengal {

inline constexpr int version_major = BENGAL_VERSION_MAJOR;
inline constexpr int version_minor = BENGAL_VERSION_MINOR;
inline constexpr int version_patch = BENGAL_VERSION_PATCH;
inline constexpr std::string_view version = BENGAL_VERSION_STRING;

}  // namespace bengal
