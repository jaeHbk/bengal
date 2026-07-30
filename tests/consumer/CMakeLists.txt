cmake_minimum_required(VERSION 3.20)

project(BengalConsumer LANGUAGES CXX)

set(
  BENGAL_SOURCE_DIR
  ""
  CACHE PATH
  "Optional Bengal source tree used for pre-release downstream validation"
)

if(BENGAL_SOURCE_DIR)
  if(NOT EXISTS "${BENGAL_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "BENGAL_SOURCE_DIR is not a Bengal source tree")
  endif()
  add_subdirectory(
    "${BENGAL_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/bengal"
  )
else()
  find_package(Bengal CONFIG REQUIRED COMPONENTS Core)
endif()

add_executable(bengal_consumer main.cpp)
target_link_libraries(bengal_consumer PRIVATE Bengal::Core)

include(CTest)
add_test(NAME bengal_consumer COMMAND bengal_consumer)
