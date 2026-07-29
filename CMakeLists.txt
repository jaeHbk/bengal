cmake_minimum_required(VERSION 3.20)

project(
  Bengal
  VERSION 0.1.0
  DESCRIPTION "Predictable-latency C++20 primitives"
  LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(bengal_core INTERFACE)
add_library(Bengal::Core ALIAS bengal_core)

set_target_properties(bengal_core PROPERTIES EXPORT_NAME Core)

target_compile_features(bengal_core INTERFACE cxx_std_20)
target_include_directories(
  bengal_core
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(BENGAL_BUILD_TESTS "Build Bengal tests" ON)
option(BENGAL_BUILD_BENCHMARKS "Build Bengal benchmarks" OFF)

if(BUILD_TESTING AND BENGAL_BUILD_TESTS)
  find_package(Threads REQUIRED)

  add_executable(bengal_tests tests/bengal_tests.cpp)
  target_link_libraries(bengal_tests PRIVATE Bengal::Core Threads::Threads)
  target_compile_options(
    bengal_tests
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
  )

  add_test(NAME bengal_tests COMMAND bengal_tests)
endif()

if(BENGAL_BUILD_BENCHMARKS)
  add_executable(bengal_benchmarks benchmarks/bengal_benchmarks.cpp)
  target_link_libraries(bengal_benchmarks PRIVATE Bengal::Core)
  target_compile_options(
    bengal_benchmarks
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
  )
endif()

install(
  TARGETS bengal_core
  EXPORT BengalTargets
)
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
  EXPORT BengalTargets
  FILE BengalTargets.cmake
  NAMESPACE Bengal::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Bengal
)

configure_package_config_file(
  cmake/BengalConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/BengalConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Bengal
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/BengalConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/BengalConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BengalConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Bengal
)
