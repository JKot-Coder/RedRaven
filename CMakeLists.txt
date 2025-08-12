cmake_minimum_required (VERSION 3.24.0)

project(RedRaven VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

enable_testing()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

message(warning ${CMAKE_BINARY_DIR})

include(FetchContent)
include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ms-extensions
if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
 add_compile_options(-fms-extensions)
 # add_compile_options(-fms-compatibility)
 add_compile_options(-Wno-language-extension-token)
 add_compile_options(-Wno-nested-anon-types)
 add_compile_options(-Wno-unused-function)
endif()

# Force MSVC to enable c++17
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_DEBUG_POSTFIX "_d")

# if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
# set(CMAKE_CXX_FLAGS_HYBRID "/O2 /Zi /Zo")
# set(CMAKE_C_FLAGS_HYBRID "/O2 /Zi /Zo")
# set(CMAKE_EXE_LINKER_FLAGS_HYBRID "/DEBUG")
# set(CMAKE_SHARED_LINKER_FLAGS_HYBRID "/DEBUG")
# else()
# set(CMAKE_CXX_FLAGS_HYBRID "-O2 -g")
# set(CMAKE_C_FLAGS_HYBRID "-O2 -g")
# set(CMAKE_EXE_LINKER_FLAGS_HYBRID "-O2 -g")
# set(CMAKE_SHARED_LINKER_FLAGS_HYBRID "-O2 -g")
# endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_compile_definitions( $<$<CONFIG:Debug>:DEBUG> )
add_compile_definitions( $<$<CONFIG:Release>:RELEASE> )
add_compile_definitions( $<$<CONFIG:RelWithDebInfo>:RELEASE_WITH_DEBUG_INFO> )
string(REPLACE "-DNDEBUG" ""
       CMAKE_CXX_FLAGS_RELWITHDEBINFO
       "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

string(REPLACE "-DNDEBUG" ""
       CMAKE_C_FLAGS_RELWITHDEBINFO
       "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
#set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

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

if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
 # if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
 #   set(COMPILE_SANITIZE_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
 #   set(LINK_SANITIZE_FLAGS "-fsanitize=address" "-shared-libasan")
 #   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
 # else()
 #   set(COMPILE_SANITIZE_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
 # endif()
endif()

add_compile_options(${COMPILE_SANITIZE_FLAGS})
add_link_options(${LINK_SANITIZE_FLAGS})

include_directories(src)

#================================ Dependencies ===============================#

add_subdirectory(src/dependencies/cr)
add_subdirectory(src/dependencies/stb)
add_subdirectory(src/dependencies/utfcpp)
add_subdirectory(src/dependencies/flecs)
add_subdirectory(src/dependencies/ska)
add_subdirectory(src/dependencies/nanobench)
#add_subdirectory(src/dependencies/imgui)

find_package(imgui CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

#=================================== Catch2 ==================================#
# FetchContent_Declare(
#   Catch2
#   GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#   GIT_TAG        182c910b4b63ff587a3440e08f84f70497e49a81 #v2.13.10
# )
# FetchContent_MakeAvailable(Catch2)
find_package(Catch2 CONFIG REQUIRED)

#=============================== ApprovalTests ===============================#
# FetchContent_Declare(
#   ApprovalTests
#   GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
#   GIT_TAG        36bea0276415b521229b0f01271601a37f72b4de #v.10.12.1
# )
# # Tell the ApprovalTests CMake files that we want to use its copy of Catch2:
# set(APPROVAL_TESTS_BUILD_THIRD_PARTY_CATCH2 ON CACHE BOOL "")
# FetchContent_MakeAvailable(ApprovalTests)
# target_compile_options(ApprovalTests PRIVATE "$<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-copy-with-dtor>" )

#=================================== Glfw ====================================#
# FetchContent_Declare(
#   Glfw
#   GIT_REPOSITORY https://github.com/glfw/glfw.git
#   GIT_TAG        45ce5ddd197d5c58f50fdd3296a5131c894e5527 #3.3.7
# )
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(Glfw)
find_package(glfw3 CONFIG REQUIRED)

#============================= DirectX-Headers ===============================#
# if(NOT WIN32)
#   FetchContent_Declare(
#     directx-headers
#     GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers
#     GIT_TAG        f149cfead6063bcf955d1a7b1ee91a6ece90f830 #v1.606.4
#     OVERRIDE_FIND_PACKAGE
#   )
#   set(DXHEADERS_INSTALL ON CACHE BOOL "" FORCE)
#   FetchContent_MakeAvailable(directx-headers)
# endif ()

# #=============================== DirectXMath =================================#
# if(NOT WIN32)
#   FetchContent_Declare(
#     directxmath
#     GIT_REPOSITORY https://github.com/microsoft/DirectXMath
#     GIT_TAG        e95d84892ec894967c48151f9106bd4388e85e63 #may2022
#     OVERRIDE_FIND_PACKAGE
#   )
#   FetchContent_MakeAvailable(directxmath)
#   add_library(Microsoft::DirectXMath ALIAS DirectXMath)
# endif ()

# #================================ DirectXTex =================================#
# FetchContent_Declare(
#   DirectXTex
#   GIT_REPOSITORY https://github.com/microsoft/DirectXTex
#   GIT_TAG        edee596475fdbec95c9d0f194036d6f190a83ba1 #apr2021
# )
# set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
# set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
# set(BUILD_DX12 OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(DirectXTex)

#================================== cxxopts ==================================#
# FetchContent_Declare(
#         cxxopts
#         GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
#         GIT_TAG        c74846a891b3cc3bfa992d588b1295f528d43039 #v3.0.0
#         GIT_SHALLOW    TRUE
# )
# set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
# set(CXXOPTS_ENABLE_WARNINGS OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(cxxopts)

#=================================== yaml ===================================#
# FetchContent_Declare(
#   yaml-cpp
#   GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
#   GIT_TAG        0579ae3d976091d7d664aa9d2527e0d0cff25763 #yaml-cpp-0.7.0
# )
# set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
# set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
# set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(yaml-cpp)

#================================== stduuid ==================================#
# FetchContent_Declare(
#   stduuid
#   GIT_REPOSITORY https://github.com/mariusbancila/stduuid
#   GIT_TAG        5c538cca02932aa0266659661d5b4726f3a317c7 #v1.2.2
# )
# set(UUID_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(stduuid)

#================================= magic_enum ================================#
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum
  GIT_TAG        67abcf483a4c5e5027aaf2976b338bd71e28c749 #v0.9.1
  UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(magic_enum)

#==================================== EASTL ==================================#
# FetchContent_Declare(
#   EASTL
#   GIT_REPOSITORY https://github.com/electronicarts/EASTL
#   GIT_TAG        e757b44f712902a78fe22886842eaba25e0a7797 #3.21.12
#   GIT_SUBMODULES ""
# )
# set(EASTL_ROOT_DIR ${EASTL_SOURCE_DIR})
# file(WRITE "${EASTL_ROOT_DIR}/test/packages/EABase/CMakeLists.txt")
# file(WRITE "${EASTL_ROOT_DIR}/source/eastl_default_allocator.cpp"
# "// AUTOGENERATED WITH CMAKE
# #include <cstdint>

# void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
# {
# 	return new uint8_t[size];
# }

# void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
# {
#     return new uint8_t[size];
# }
# ")
# FetchContent_MakeAvailable(EASTL)
find_package(EASTL CONFIG REQUIRED)

#================================== EABase ===================================#
# FetchContent_Declare(
#   EABase
#   GIT_REPOSITORY https://github.com/electronicarts/EABase
#   GIT_TAG        521cb053d
#   GIT_SUBMODULES ""
# )
# FetchContent_MakeAvailable(EABase)

#==================================== Fmt ====================================#
FetchContent_Declare(
  FMT
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281 #10.2.1
  UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(FMT)
target_compile_definitions(fmt PUBLIC FMT_STATIC_THOUSANDS_SEPARATOR=1)

#=================================== Flecs ===================================#
# FetchContent_Declare(
#   flecs
#   GIT_REPOSITORY https://github.com/SanderMertens/flecs
#   GIT_TAG        198607d10ab8f8fc44540043271d6e3be019250b #v3.2.11
# )
# FetchContent_MakeAvailable(flecs)
# find_package(flecs CONFIG REQUIRED)

#==================================== efsw ===================================#
# FetchContent_Declare(
#   efsw
#   GIT_REPOSITORY https://github.com/SpartanJ/efsw
#   GIT_TAG        3b0ffd2908fa8d52a54e228f92c67330dfd015b4 #1.3.1
# )
# FetchContent_MakeAvailable(efsw)
# set(BuildTests OFF CACHE BOOL "" FORCE)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")
# set_property(TARGET Catch2 PROPERTY FOLDER "Dependencies")
# set_property(TARGET ApprovalTests PROPERTY FOLDER "Dependencies")
# set_property(TARGET glfw PROPERTY FOLDER "Dependencies")
set_property(TARGET imgui::imgui PROPERTY FOLDER "Dependencies")
#set_property(TARGET DirectXTex PROPERTY FOLDER "Dependencies")
# set_property(TARGET cxxopts PROPERTY FOLDER "Dependencies")
# set_property(TARGET yaml-cpp PROPERTY FOLDER "Dependencies")
# set_property(TARGET magic_enum PROPERTY FOLDER "Dependencies")
# set_property(TARGET EASTL PROPERTY FOLDER "Dependencies")
# set_property(TARGET flecs PROPERTY FOLDER "Dependencies")
# set_property(TARGET efsw PROPERTY FOLDER "Dependencies")

add_subdirectory(src)