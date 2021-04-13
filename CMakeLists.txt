cmake_minimum_required (VERSION 3.16.0)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

message(warning ${CMAKE_BINARY_DIR})

include(FetchContent)

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
FetchContent_Declare(
  DirectXTex
  GIT_REPOSITORY https://github.com/microsoft/DirectXTex
  GIT_TAG        apr2021
)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(DirectXTex)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET STB PROPERTY FOLDER "Dependencies")
set_property(TARGET fmt PROPERTY FOLDER "Dependencies")
set_property(TARGET backward PROPERTY FOLDER "Dependencies")
set_property(TARGET backward_object PROPERTY FOLDER "Dependencies")
set_property(TARGET Catch2WithMain PROPERTY FOLDER "Dependencies")
set_property(TARGET ApprovalTests PROPERTY FOLDER "Dependencies")
set_property(TARGET glfw PROPERTY FOLDER "Dependencies")
set_property(TARGET DirectXTex PROPERTY FOLDER "Dependencies")

add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gapi_dx12)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/render)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/common)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/windowing)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/inputting)

add_subdirectory(src/open_demo)
add_subdirectory(src/tests)
##add_subdirectory(src)