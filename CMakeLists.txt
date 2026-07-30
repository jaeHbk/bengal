cmake_minimum_required(VERSION 3.20)

project(
  Bengal
  VERSION 0.3.0
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
option(
  BENGAL_BUILD_GOOGLE_BENCHMARKS
  "Build Bengal Google Benchmark cross-checks"
  OFF
)
option(BENGAL_BUILD_FUZZERS "Build Bengal libFuzzer targets" OFF)
option(BENGAL_BUILD_EXAMPLES "Build Bengal examples" ON)

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

  add_executable(bengal_spsc_stress tests/spsc_queue_stress.cpp)
  target_link_libraries(
    bengal_spsc_stress
    PRIVATE Bengal::Core Threads::Threads
  )
  target_compile_options(
    bengal_spsc_stress
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
  )
  add_test(NAME bengal_spsc_stress COMMAND bengal_spsc_stress)
  set_tests_properties(bengal_spsc_stress PROPERTIES TIMEOUT 30)
endif()

if(BENGAL_BUILD_BENCHMARKS)
  find_package(Threads REQUIRED)

  add_executable(bengal_benchmarks benchmarks/bengal_benchmarks.cpp)
  target_link_libraries(
    bengal_benchmarks
    PRIVATE
      Bengal::Core
      Threads::Threads
  )
  target_compile_options(
    bengal_benchmarks
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
  )
endif()

if(BENGAL_BUILD_GOOGLE_BENCHMARKS)
  find_package(benchmark CONFIG REQUIRED)
  find_package(Threads REQUIRED)

  add_executable(
    bengal_google_benchmarks
    benchmarks/google_benchmarks.cpp
  )
  target_link_libraries(
    bengal_google_benchmarks
    PRIVATE
      Bengal::Core
      Threads::Threads
      benchmark::benchmark
  )
endif()

if(BENGAL_BUILD_FUZZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Bengal fuzzers currently require Clang")
  endif()

  add_executable(
    bengal_short_string_fuzz
    fuzz/short_string_fuzz.cpp
  )
  target_link_libraries(bengal_short_string_fuzz PRIVATE Bengal::Core)
  target_compile_options(
    bengal_short_string_fuzz
    PRIVATE
      -fsanitize=fuzzer,address,undefined
      -fno-omit-frame-pointer
  )
  target_link_options(
    bengal_short_string_fuzz
    PRIVATE
      -fsanitize=fuzzer,address,undefined
  )

  add_executable(
    bengal_static_buffer_resource_fuzz
    fuzz/static_buffer_resource_fuzz.cpp
  )
  target_link_libraries(
    bengal_static_buffer_resource_fuzz
    PRIVATE
      Bengal::Core
  )
  target_compile_options(
    bengal_static_buffer_resource_fuzz
    PRIVATE
      -fsanitize=fuzzer,address,undefined
      -fno-omit-frame-pointer
  )
  target_link_options(
    bengal_static_buffer_resource_fuzz
    PRIVATE
      -fsanitize=fuzzer,address,undefined
  )
endif()

if(BENGAL_BUILD_EXAMPLES)
  find_package(Threads REQUIRED)

  add_executable(
    bengal_bounded_pipeline
    examples/bounded_pipeline.cpp
  )
  target_link_libraries(
    bengal_bounded_pipeline
    PRIVATE
      Bengal::Core
      Threads::Threads
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
