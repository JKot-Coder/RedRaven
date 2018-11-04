set(definitions)

set(RENDER AUTO CACHE STRING "Graphics api")
set_property(CACHE RENDER PROPERTY STRINGS OPENGL)

if (WIN32)
    set(OPEN_DEMO_SRC_PLATFORM)
    SET(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_WIN
            NOMINMAX)
    if(RENDER STREQUAL AUTO)
        SET(RENDER OPENGL)
    endif()
endif ()

if (APPLE)
    set(OPEN_DEMO_SRC_PLATFORM)
    SET(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_APPLE)
    if(RENDER STREQUAL AUTO)
        SET(RENDER OPENGL)
    endif()
endif ()

if (LINUX)
    set(OPEN_DEMO_SRC_PLATFORM)
    if(RENDER STREQUAL AUTO)
        SET(RENDER OPENGL)
    endif()
endif ()

if (RENDER STREQUAL OPENGL)
    find_package(OpenGL REQUIRED)

    set(OPENGL_SOURCES
            render/render_gl.hpp)

    set(RENDER_DEFINITIONS
            RENDER_GL)
endif ()

find_package(SDL2 REQUIRED)

set(OPEN_DEMO_LINK_LIBRARIES
        ${SDL2_LIBRARY}
        ${OPENGL_LIBRARIES})

set(OPEN_DEMO_DEFINITIONS
        ${OPEN_DEMO_PLATFORM_DEFINITIONS} )

set(OPEN_DEMO_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/windowing
        ${SDL2_INCLUDE_DIR})

set(OPEN_DEMO_SRC_COMMON
        input.h
        vecmath.h
        main.cpp
        Application.cpp
        Application.hpp)

set(OPEN_DEMO_SRC_MODULE_COMMON
        common/Exception.cpp
        common/Exception.hpp)

set(OPEN_DEMO_SRC_MODULE_WINDOWING
        windowing/Listener.hpp
        windowing/Window.hpp
        windowing/Windowing.hpp
        windowing/WindowSettings.hpp
        windowing/Windowing.cpp
        windowing/Window.cpp)

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_COMMON}
        ${OPEN_DEMO_SRC_PLATFORM}
        ${OPEN_DEMO_SRC_MODULE_COMMON}
        ${OPEN_DEMO_SRC_MODULE_WINDOWING})

add_executable (open_demo ${OPEN_DEMO_SRC})
target_link_libraries(open_demo ${OPEN_DEMO_LINK_LIBRARIES})
target_compile_definitions(open_demo PRIVATE ${OPEN_DEMO_DEFINITIONS})
target_include_directories(open_demo PRIVATE ${OPEN_DEMO_INCLUDE_DIRS})