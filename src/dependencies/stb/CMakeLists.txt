cmake_minimum_required (VERSION 3.2.2)
project(STB)

add_library(STB STATIC ${PROJECT_SOURCE_DIR}/stb_image.h ${PROJECT_SOURCE_DIR}/stb_image.c)
target_include_directories(STB PUBLIC ${PROJECT_SOURCE_DIR})
set_target_properties(STB PROPERTIES LINKER_LANGUAGE C)