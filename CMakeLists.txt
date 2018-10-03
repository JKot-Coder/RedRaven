cmake_minimum_required (VERSION 3.2.2)
project (OpenDemo)

set (CMAKE_CXX_STANDARD 11)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

add_subdirectory(src)