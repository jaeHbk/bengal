cmake_minimum_required(VERSION 3.20)

project(BengalConsumer LANGUAGES CXX)

find_package(Bengal CONFIG REQUIRED)

add_executable(bengal_consumer main.cpp)
target_link_libraries(bengal_consumer PRIVATE Bengal::Core)

