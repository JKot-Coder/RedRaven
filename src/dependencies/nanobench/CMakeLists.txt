cmake_minimum_required(VERSION 3.8)
project(nanobench LANGUAGES CXX)

add_library(nanobench STATIC nanobench.cpp)
add_library(nanobench::nanobench ALIAS nanobench)
set_property(TARGET nanobench PROPERTY CXX_STANDARD 17)
target_include_directories(nanobench PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
