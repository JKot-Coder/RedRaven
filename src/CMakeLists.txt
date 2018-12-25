set(definitions)


set(rendering AUTO CACHE STRING "Graphics api")
set_property(CACHE rendering PROPERTY STRINGS OPENGL)

set(OPEN_DEMO_BUILD_DEFINITIONS)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(OPEN_DEMO_BUILD_DEFINITIONS
            DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(OPEN_DEMO_BUILD_DEFINITIONS
            RELEASE)
endif()

if (WIN32)
    set(OPEN_DEMO_SRC_PLATFORM)
    SET(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_WIN
            NOMINMAX)
    if(rendering STREQUAL AUTO)
        SET(rendering OPENGL)
    endif()
endif ()

if (APPLE)
    set(OPEN_DEMO_SRC_PLATFORM)
    SET(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_APPLE)
    if(rendering STREQUAL AUTO)
        SET(rendering OPENGL)
    endif()
endif ()

if (LINUX)
    set(OPEN_DEMO_SRC_PLATFORM)
    if(rendering STREQUAL AUTO)
        SET(rendering OPENGL)
    endif()
endif ()

if (rendering STREQUAL OPENGL)
    find_package(OpenGL REQUIRED)
    set(RENDER_GAPI_LIBRARIES ${OPENGL_LIBRARIES})

    set(OPEN_DEMO_SRC_MODULE_RENDER_GAPI
            rendering/opengl/Shader.cpp
            rendering/opengl/Shader.hpp
            rendering/opengl/Render.hpp
            rendering/opengl/Render.cpp
            rendering/opengl/Mesh.cpp
            rendering/opengl/Mesh.hpp
            rendering/opengl/Texture.cpp
            rendering/opengl/Texture.hpp
            rendering/opengl/RenderTargetContext.cpp
            rendering/opengl/RenderTargetContext.hpp)

    set(OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI
            ${CMAKE_SOURCE_DIR}/src/render/opengl
            ${CMAKE_SOURCE_DIR}/src/depencies/glad
            ${CMAKE_SOURCE_DIR}/src/depencies/KHR)
endif ()

find_package(SDL2 REQUIRED)

set(OPEN_DEMO_LINK_LIBRARIES
        ${SDL2_LIBRARY}
        ${RENDER_GAPI_LIBRARIES})

set(OPEN_DEMO_DEFINITIONS
        ${OPEN_DEMO_PLATFORM_DEFINITIONS}
        ${OPEN_DEMO_BUILD_DEFINITIONS})

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
        common/Utils.hpp
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
        rendering/Render.cpp
        rendering/Render.hpp
        rendering/Shader.hpp
        rendering/Mesh.hpp
        rendering/Primitives.cpp
        rendering/Primitives.hpp
        rendering/Shader.cpp
        rendering/RenderTargetContext.hpp
        rendering/RenderPasses.cpp
        rendering/RenderPasses.hpp
        rendering/RenderTarget.hpp
        rendering/Texture.hpp
        rendering/SceneGraph.hpp
        rendering/RenderContext.hpp
        rendering/Material.hpp
        rendering/Camera.hpp
        rendering/Transform.hpp
        rendering/RenderContext.cpp)

set(OPEN_DEMO_SRC_SCENES
        scenes/Scene_1.hpp
        scenes/Scene_1.cpp)

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_DEPS_GLAD}
        ${OPEN_DEMO_SRC_COMMON}
        ${OPEN_DEMO_SRC_PLATFORM}
        ${OPEN_DEMO_SRC_MODULE_COMMON}
        ${OPEN_DEMO_SRC_MODULE_FILE_SYSTEM}
        ${OPEN_DEMO_SRC_MODULE_RESOURCE_MANAGER}
        ${OPEN_DEMO_SRC_MODULE_WINDOWING}
        ${OPEN_DEMO_SRC_MODULE_RENDER}
        ${OPEN_DEMO_SRC_MODULE_RENDER_GAPI}
        ${OPEN_DEMO_SRC_SCENES})

add_executable(open_demo ${OPEN_DEMO_SRC})
target_link_libraries(open_demo ${OPEN_DEMO_LINK_LIBRARIES})
target_compile_definitions(open_demo PRIVATE ${OPEN_DEMO_DEFINITIONS})
target_include_directories(open_demo PRIVATE ${OPEN_DEMO_INCLUDE_DIRS})

if(MSVC)
    target_compile_options($open_demo PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(open_demo PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)