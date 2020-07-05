cmake_minimum_required(VERSION 3.1.0)

project(eigen)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE ${PROJECT_SOURCE_DIR})
target_compile_definitions(eigen INTERFACE -DEIGEN_PERMANENTLY_DISABLE_STUPID_WARNINGS)
