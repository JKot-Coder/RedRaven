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

    set(OPEN_DEMO_SRC_MODULE_RENDER_GAPI
            render/opengl/Shader.cpp
            render/opengl/Shader.hpp
            render/opengl/Rendering.hpp
            render/opengl/Rendering.cpp
            render/opengl/Mesh.cpp
            render/opengl/Mesh.hpp)

    set(OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI
            ${CMAKE_SOURCE_DIR}/src/render/opengl
            ${CMAKE_SOURCE_DIR}/src/depencies/glad
            ${CMAKE_SOURCE_DIR}/src/depencies/KHR)
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
        ${CMAKE_SOURCE_DIR}/src/filesystem
        ${CMAKE_SOURCE_DIR}/src/resource_manager
        ${CMAKE_SOURCE_DIR}/src/windowing
        ${CMAKE_SOURCE_DIR}/src/render
        ${OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI}
        ${SDL2_INCLUDE_DIR})

set(OPEN_DEMO_SRC_COMMON
        input.h
        common/VecMath.h
        main.cpp
        Application.cpp
        Application.hpp)

set(OPEN_DEMO_SRC_DEPS_GLAD
        dependencies/glad/glad.c
        dependencies/glad/glad.h
        dependencies/KHR/khrplatform.h)

set(OPEN_DEMO_SRC_MODULE_COMMON
        common/Exception.cpp
        common/Exception.hpp
        common/Stream.hpp)

set(OPEN_DEMO_SRC_MODULE_FILE_SYSTEM
        filesystem/FileSystem.cpp
        filesystem/FileSystem.hpp
        filesystem/FileStream.cpp
        filesystem/FileStream.hpp)

set(OPEN_DEMO_SRC_MODULE_RESOURCE_MANAGER
        resource_manager/ResourceManager.cpp
        resource_manager/ResourceManager.hpp)

set(OPEN_DEMO_SRC_MODULE_WINDOWING
        windowing/Listener.hpp
        windowing/Window.hpp
        windowing/Windowing.hpp
        windowing/WindowSettings.hpp
        windowing/Windowing.cpp
        windowing/Window.cpp)

set(OPEN_DEMO_SRC_MODULE_RENDER
        render/Rendering.hpp
        render/Shader.hpp
        render/Mesh.hpp)

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_DEPS_GLAD}
        ${OPEN_DEMO_SRC_COMMON}
        ${OPEN_DEMO_SRC_PLATFORM}
        ${OPEN_DEMO_SRC_MODULE_COMMON}
        ${OPEN_DEMO_SRC_MODULE_FILE_SYSTEM}
        ${OPEN_DEMO_SRC_MODULE_RESOURCE_MANAGER}
        ${OPEN_DEMO_SRC_MODULE_WINDOWING}
        ${OPEN_DEMO_SRC_MODULE_RENDER}
        ${OPEN_DEMO_SRC_MODULE_RENDER_GAPI} common/Utils.hpp)

add_executable (open_demo ${OPEN_DEMO_SRC})
target_link_libraries(open_demo ${OPEN_DEMO_LINK_LIBRARIES})
target_compile_definitions(open_demo PRIVATE ${OPEN_DEMO_DEFINITIONS})
target_include_directories(open_demo PRIVATE ${OPEN_DEMO_INCLUDE_DIRS})

if(MSVC)
    target_compile_options($open_demo PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(open_demo PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)