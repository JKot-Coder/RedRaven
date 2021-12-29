cmake_minimum_required (VERSION 3.16.0)

project(RedRaven VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

message(warning ${CMAKE_BINARY_DIR})

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Force MSVC to enable c++17 
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_DEBUG_POSTFIX "_d")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
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

set(PLATFORM_DEFINITIONS)

if (WIN32)
    set(PLATFORM_DEFINITIONS -DOS_WINDOWS -DNOMINMAX -DUNICODE -D_UNICODE)
endif ()

if (APPLE)
    set(PLATFORM_DEFINITIONS -DOS_APPLE)
endif ()

if (LINUX)
    set(PLATFORM_DEFINITIONS -DOS_LINUX)
endif ()
                        
add_definitions(${PLATFORM_DEFINITIONS})

include_directories(src)

add_subdirectory(src/dependencies/stb)
add_subdirectory(src/dependencies/gulrak/filesystem)
add_subdirectory(src/dependencies/fmt)
add_subdirectory(src/dependencies/utfcpp)
add_subdirectory(src/dependencies/backward-cpp)

#=================================== Catch2 ==================================#
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.4
)
FetchContent_MakeAvailable(Catch2)

#=============================== ApprovalTests ===============================#
FetchContent_Declare(
  ApprovalTests
  GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
  GIT_TAG        v.10.6.0
)
# Tell the ApprovalTests CMake files that we want to use its copy of Catch2:
set(APPROVAL_TESTS_BUILD_THIRD_PARTY_CATCH2 ON CACHE BOOL "")
FetchContent_MakeAvailable(ApprovalTests)

#=================================== Glfw ====================================#
FetchContent_Declare(
  Glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG        3.3.3
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Glfw)

#================================ DirectXTex =================================#
if(FALSE)
FetchContent_Declare(
  DirectXTex
  GIT_REPOSITORY https://github.com/microsoft/DirectXTex
  GIT_TAG        apr2021
)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(DirectXTex)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")
set_property(TARGET backward PROPERTY FOLDER "Dependencies")
set_property(TARGET backward_object PROPERTY FOLDER "Dependencies")
set_property(TARGET Catch2WithMain PROPERTY FOLDER "Dependencies")
set_property(TARGET ApprovalTests PROPERTY FOLDER "Dependencies")
set_property(TARGET glfw PROPERTY FOLDER "Dependencies")
#set_property(TARGET DirectXTex PROPERTY FOLDER "Dependencies")

add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi)
if (WIN32)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi_dx12)
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/render)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/common)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/windowing)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/inputting)

add_subdirectory(src/demo)
add_subdirectory(src/tests)
add_subdirectory(src/rfx)