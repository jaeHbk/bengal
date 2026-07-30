cmake_minimum_required(VERSION 3.20)

project(
  Bengal
  VERSION 1.0.0
  DESCRIPTION "Predictable-latency C++20 primitives"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(BENGAL_DEVELOPER_DEFAULT ON)
else()
  set(BENGAL_DEVELOPER_DEFAULT OFF)
endif()

find_package(Threads REQUIRED)

add_library(bengal_core INTERFACE)
add_library(Bengal::Core ALIAS bengal_core)

set_target_properties(bengal_core PROPERTIES EXPORT_NAME Core)

target_compile_features(bengal_core INTERFACE cxx_std_20)
target_link_libraries(bengal_core INTERFACE Threads::Threads)
target_include_directories(
  bengal_core
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(
  BENGAL_BUILD_TESTS
  "Build Bengal tests"
  ${BENGAL_DEVELOPER_DEFAULT}
)
option(BENGAL_BUILD_BENCHMARKS "Build Bengal benchmarks" OFF)
option(
  BENGAL_BUILD_GOOGLE_BENCHMARKS
  "Build Bengal Google Benchmark cross-checks"
  OFF
)
option(BENGAL_BUILD_FUZZERS "Build Bengal libFuzzer targets" OFF)
option(
  BENGAL_BUILD_EXAMPLES
  "Build Bengal examples"
  ${BENGAL_DEVELOPER_DEFAULT}
)
option(
  BENGAL_INSTALL
  "Generate Bengal installation and package targets"
  ${BENGAL_DEVELOPER_DEFAULT}
)

function(bengal_enable_warnings target)
  target_compile_options(
    ${target}
    PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
      $<$<CXX_COMPILER_ID:MSVC>:/W4;/WX;/permissive-;/wd4324>
  )
endfunction()

if(BENGAL_BUILD_TESTS)
  include(CTest)
endif()

if(BENGAL_BUILD_TESTS AND BUILD_TESTING)
  add_executable(bengal_tests tests/bengal_tests.cpp)
  target_link_libraries(bengal_tests PRIVATE Bengal::Core Threads::Threads)
  bengal_enable_warnings(bengal_tests)

  add_test(NAME bengal_tests COMMAND bengal_tests)

  add_executable(bengal_spsc_stress tests/spsc_queue_stress.cpp)
  target_link_libraries(
    bengal_spsc_stress
    PRIVATE Bengal::Core Threads::Threads
  )
  bengal_enable_warnings(bengal_spsc_stress)
  add_test(NAME bengal_spsc_stress COMMAND bengal_spsc_stress)
  set_tests_properties(bengal_spsc_stress PROPERTIES TIMEOUT 30)

  add_executable(bengal_api_contract tests/api_contract.cpp)
  target_link_libraries(bengal_api_contract PRIVATE Bengal::Core)
  bengal_enable_warnings(bengal_api_contract)
  add_test(NAME bengal_api_contract COMMAND bengal_api_contract)

  add_library(
    bengal_public_header_checks
    OBJECT
      tests/headers/bengal.cpp
      tests/headers/concurrency_spsc_queue.cpp
      tests/headers/memory_static_buffer_resource.cpp
      tests/headers/meta_type_map.cpp
      tests/headers/meta_type_set.cpp
      tests/headers/platform_capabilities.cpp
      tests/headers/text_short_string.cpp
      tests/headers/thread_qos_jthread.cpp
      tests/headers/version.cpp
  )
  target_link_libraries(bengal_public_header_checks PRIVATE Bengal::Core)
  bengal_enable_warnings(bengal_public_header_checks)
endif()

if(BENGAL_BUILD_BENCHMARKS)
  add_executable(bengal_benchmarks benchmarks/bengal_benchmarks.cpp)
  target_link_libraries(
    bengal_benchmarks
    PRIVATE
      Bengal::Core
      Threads::Threads
  )
  bengal_enable_warnings(bengal_benchmarks)
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
  bengal_enable_warnings(bengal_bounded_pipeline)
endif()

if(BENGAL_INSTALL)
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
  if(PROJECT_VERSION_MAJOR EQUAL 0)
    set(BENGAL_PACKAGE_COMPATIBILITY SameMinorVersion)
  else()
    set(BENGAL_PACKAGE_COMPATIBILITY SameMajorVersion)
  endif()
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/BengalConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ${BENGAL_PACKAGE_COMPATIBILITY}
  )
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/BengalConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/BengalConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Bengal
  )
endif()
