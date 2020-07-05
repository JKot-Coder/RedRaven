cmake_minimum_required (VERSION 3.16.0)

project(OpenDemo)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

message(warning ${CMAKE_BINARY_DIR})
include("build/conanbuildinfo_multi.cmake")
conan_basic_setup(TARGETS)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set (CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_compile_definitions( $<$<CONFIG:Debug>:DEBUG>)
add_compile_definitions( $<$<CONFIG:Release>:RELEASE> )

set(OPEN_DEMO_PLATFORM_DEFINITIONS)

if (WIN32)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_WINDOWS -DNOMINMAX)
endif ()

if (APPLE)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_APPLE)
endif ()

if (LINUX)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_LINUX)
endif ()
                        
add_definitions(${OPEN_DEMO_PLATFORM_DEFINITIONS})

include_directories(src)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src/dependencies/stb)
add_subdirectory(src/dependencies/gulrak/filesystem)
add_subdirectory(src/dependencies/fmt)
add_subdirectory(src/dependencies/utfcpp)
add_subdirectory(src/dependencies/eigen)

set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")

add_subdirectory(src)