cmake_minimum_required (VERSION 3.16.0)

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
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_WINDOWS -DNOMINMAX -DUNICODE -D_UNICODE)
endif ()

if (APPLE)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_APPLE)
endif ()

if (LINUX)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS -DOS_LINUX)
endif ()
                        
add_definitions(${OPEN_DEMO_PLATFORM_DEFINITIONS})

include_directories(src)

add_subdirectory(src/dependencies/stb)
add_subdirectory(src/dependencies/gulrak/filesystem)
add_subdirectory(src/dependencies/fmt)
add_subdirectory(src/dependencies/utfcpp)
add_subdirectory(src/dependencies/eigen)
add_subdirectory(src/dependencies/backward-cpp)
add_subdirectory(src/dependencies/ut)
add_subdirectory(src/dependencies/catch2)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")
set_property(TARGET backward PROPERTY FOLDER "Dependencies")
set_property(TARGET backward_object PROPERTY FOLDER "Dependencies")
set_property(TARGET style PROPERTY FOLDER "Dependencies")
set_property(TARGET Catch2 PROPERTY FOLDER "Dependencies")
set_property(TARGET Catch2WithMain PROPERTY FOLDER "Dependencies")

add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi_dx12)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/render)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/common)

add_subdirectory(src/open_demo)
add_subdirectory(src/tests)
##add_subdirectory(src)