project (renderer)

set(GAPI AUTO CACHE STRING "Graphics api")
set_property(CACHE GAPI PROPERTY STRINGS OPENGL)

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
    set(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_WINDOWS
            NOMINMAX)
    if(GAPI STREQUAL AUTO)
        set(GAPI OPENGL)
    endif()
endif ()

if (APPLE)
    set(OPEN_DEMO_SRC_PLATFORM)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_APPLE)
    if(GAPI STREQUAL AUTO)
        set(GAPI OPENGL)
    endif()
endif ()

if (LINUX)
    set(OPEN_DEMO_SRC_PLATFORM)
    if(GAPI STREQUAL AUTO)
        set(GAPI OPENGL)
    endif()
endif ()

if (GAPI STREQUAL OPENGL)
    find_package(OpenGL REQUIRED)
    set(RENDER_GAPI_LIBRARIES ${OPENGL_LIBRARIES} CONAN_PKG::sdl2)

    set(OPEN_DEMO_SRC_MODULE_RENDER_GAPI
            opengl/Shader.cpp
            opengl/Shader.hpp
            opengl/Render.hpp
            opengl/Render.cpp
            opengl/Mesh.cpp
            opengl/Mesh.hpp
            opengl/Texture.cpp
            opengl/Texture.hpp
            opengl/RenderTargetContext.cpp
            opengl/RenderTargetContext.hpp)

    set(OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI
            ${PROJECT_SOURCE_DIR}/opengl
            ${PROJECT_SOURCE_DIR}/opengl/glad
            ${PROJECT_SOURCE_DIR}/opengl/KHR)
endif ()

set(OPEN_DEMO_SRC_DEPS_GLAD
        opengl/glad/glad.c
        opengl/glad/glad.h
        opengl/KHR/khrplatform.h)

set(OPEN_DEMO_SRC_MODULE_RENDER
        Render.hpp
        Shader.hpp
        Mesh.hpp
        Primitives.cpp
        Primitives.hpp
        Shader.cpp
        RenderTargetContext.hpp
        RenderPasses.cpp
        RenderPasses.hpp
        RenderTarget.hpp
        Texture.hpp
        SceneGraph.hpp
        RenderContext.hpp
        Material.hpp
        Camera.hpp
        Transform.hpp
        RenderContext.cpp
        RenderPipeline.cpp
        RenderPipeline.hpp
        BlendingDescription.hpp
        DepthDescription.hpp)

set(OPEN_DEMO_INCLUDE_DIRS ".." ${OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI})

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_DEPS_GLAD}
        ${OPEN_DEMO_SRC_MODULE_WINDOWING}
        ${OPEN_DEMO_SRC_MODULE_RENDER}
        ${OPEN_DEMO_SRC_MODULE_RENDER_GAPI})

set(OPEN_DEMO_LINK_LIBRARIES ${RENDER_GAPI_LIBRARIES})

add_library(${PROJECT_NAME} ${OPEN_DEMO_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} ${OPEN_DEMO_LINK_LIBRARIES} common)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPEN_DEMO_INCLUDE_DIRS})

#if(MSVC)
#	set_target_properties(open_demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "../build/bin/")
#    target_compile_options(open_demo PRIVATE /W4 /WX)
#else(MSVC)
#    target_compile_options(open_demo PRIVATE -Wall -Wextra -pedantic -Werror)
#endif(MSVC)