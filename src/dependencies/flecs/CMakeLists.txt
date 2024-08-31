cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0063 NEW)

project(flecs LANGUAGES C)

option(FLECS_STATIC "Build static flecs lib" OFF)
option(FLECS_SHARED "Build shared flecs lib" ON)
option(FLECS_PIC "Compile static flecs lib with position independent code (PIC)" ON)

set(INC flecs.h)
set(SRC flecs.c)

set(FLECS_TARGETS "")

macro(add_flecs_target TARGET CONFIG)
    add_library(${TARGET} ${CONFIG} ${INC} ${SRC})
    add_library(flecs::${TARGET} ALIAS ${TARGET})

    if(WIN32)
        target_link_libraries(${TARGET} wsock32 ws2_32)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${TARGET} pthread)
    endif()

    if(FLECS_PIC)
        set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()

    target_compile_definitions(${TARGET} PUBLIC FLECS_CONFIG_HEADER)
    target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)

    list(APPEND FLECS_TARGETS ${TARGET})
endmacro()

if(FLECS_SHARED)
    add_flecs_target(flecs SHARED)
    target_compile_definitions(flecs PRIVATE flecs_EXPORTS)
endif()

if(FLECS_STATIC)
    add_flecs_target(flecs_static STATIC)
    target_compile_definitions(flecs_static PUBLIC flecs_STATIC)
endif()

message(STATUS "Targets: ${FLECS_TARGETS}")