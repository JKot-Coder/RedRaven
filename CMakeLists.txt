cmake_minimum_required (VERSION 3.24.0)

project(RedRaven VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

enable_testing()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

message(warning ${CMAKE_BINARY_DIR})

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# ms-extensions
if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
  add_compile_options(-fms-extensions)
  add_compile_options(-Wno-language-extension-token)
endif()

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

add_compile_definitions( $<$<CONFIG:Debug>:DEBUG> )
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

set(SANITIZE_FLAGS "$<$<CONFIG:Debug>:-fsanitize=address>")
add_compile_options(${SANITIZE_FLAGS})
#target_compile_options(fmt PRIVATE {SANITIZE_FLAGS})

include_directories(src)


#================================ Dependencies ===============================#

add_subdirectory(src/dependencies/stb)
add_subdirectory(src/dependencies/gulrak/filesystem)
add_subdirectory(src/dependencies/fmt)
add_subdirectory(src/dependencies/utfcpp)
add_subdirectory(src/dependencies/backward-cpp)
add_subdirectory(src/dependencies/imgui)

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
  GIT_TAG        v.10.12.1
)
# Tell the ApprovalTests CMake files that we want to use its copy of Catch2:
set(APPROVAL_TESTS_BUILD_THIRD_PARTY_CATCH2 ON CACHE BOOL "")
FetchContent_MakeAvailable(ApprovalTests)

#=================================== Glfw ====================================#
FetchContent_Declare(
  Glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.7
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Glfw)

#============================= DirectX-Headers ===============================#
if(NOT WIN32)
  FetchContent_Declare(
    directx-headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers
    GIT_TAG        v1.606.4
    OVERRIDE_FIND_PACKAGE
  )
  set(DXHEADERS_INSTALL ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(directx-headers)
endif ()

#=============================== DirectXMath =================================#
if(NOT WIN32)
  FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath
    GIT_TAG        may2022
    OVERRIDE_FIND_PACKAGE
  )
  FetchContent_MakeAvailable(directxmath)
  add_library(Microsoft::DirectXMath ALIAS DirectXMath)
endif ()

#================================ DirectXTex =================================#
FetchContent_Declare(
  DirectXTex
  GIT_REPOSITORY https://github.com/microsoft/DirectXTex
  GIT_TAG        apr2021
)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
set(BUILD_DX12 OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(DirectXTex)

#================================== cxxopts ==================================#
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG        v3.0.0
        GIT_SHALLOW    TRUE
)
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(CXXOPTS_ENABLE_WARNINGS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cxxopts)

#================================== cxxopts ==================================#
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        yaml-cpp-0.7.0
)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

#================================== stduuid ==================================#
FetchContent_Declare(
  stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid
  GIT_TAG        v1.2.2
)
set(UUID_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(stduuid)

#================================= magic_enum ================================#
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum
  GIT_TAG        v0.9.1
)
FetchContent_MakeAvailable(magic_enum)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")
set_property(TARGET backward PROPERTY FOLDER "Dependencies")
set_property(TARGET backward_object PROPERTY FOLDER "Dependencies")
set_property(TARGET Catch2WithMain PROPERTY FOLDER "Dependencies")
set_property(TARGET ApprovalTests PROPERTY FOLDER "Dependencies")
set_property(TARGET glfw PROPERTY FOLDER "Dependencies")
set_property(TARGET imgui PROPERTY FOLDER "Dependencies")
set_property(TARGET DirectXTex PROPERTY FOLDER "Dependencies")
set_property(TARGET cxxopts PROPERTY FOLDER "Dependencies")
set_property(TARGET yaml-cpp PROPERTY FOLDER "Dependencies")
set_property(TARGET magic_enum PROPERTY FOLDER "Dependencies")



add_subdirectory(src)