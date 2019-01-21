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
    set(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_WIN
            NOMINMAX)
    if(rendering STREQUAL AUTO)
        set(rendering OPENGL)
    endif()
endif ()

if (APPLE)
    set(OPEN_DEMO_SRC_PLATFORM)
    set(OPEN_DEMO_PLATFORM_DEFINITIONS
            OS_APPLE)
    if(rendering STREQUAL AUTO)
        set(rendering OPENGL)
    endif()
endif ()

if (LINUX)
    set(OPEN_DEMO_SRC_PLATFORM)
    if(rendering STREQUAL AUTO)
        set(rendering OPENGL)
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
            ${PROJECT_SOURCE_DIR}/src/render/opengl
            ${PROJECT_SOURCE_DIR}/src/depencies/glad
            ${PROJECT_SOURCE_DIR}/src/depencies/KHR)
endif ()

set(OPEN_DEMO_LINK_LIBRARIES
        ${SDL2_LIBRARY}
        ${RENDER_GAPI_LIBRARIES}
        STB
        ${CONAN_LIBS_ASSIMP}
        ${CONAN_LIBS_BOOST_FILESYSTEM}
        ${CONAN_LIBS})

set(OPEN_DEMO_DEFINITIONS
        ${OPEN_DEMO_PLATFORM_DEFINITIONS}
        ${OPEN_DEMO_BUILD_DEFINITIONS})

set(OPEN_DEMO_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/common
        ${PROJECT_SOURCE_DIR}/src/filesystem
        ${PROJECT_SOURCE_DIR}/src/input
        ${PROJECT_SOURCE_DIR}/src/resource_manager
        ${PROJECT_SOURCE_DIR}/src/windowing
        ${PROJECT_SOURCE_DIR}/src/render
        ${OPEN_DEMO_INCLUDE_DIRS_MODULE_RENDER_GAPI}
        ${SDL2_INCLUDE_DIR})

set(OPEN_DEMO_SRC_COMMON
        inputting/Input.hpp
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
        common/Stream.hpp
        common/Time.cpp
        common/Time.hpp)

set(OPEN_DEMO_SRC_MODULE_FILE_SYSTEM
        filesystem/FileSystem.cpp
        filesystem/FileSystem.hpp
        filesystem/FileStream.cpp
        filesystem/FileStream.hpp)

set(OPEN_DEMO_SRC_MODULE_INPUTTING
        inputting/Input.hpp
        inputting/Input.cpp)

set(OPEN_DEMO_SRC_MODULE_RESOURCE_MANAGER
        resource_manager/ResourceManager.cpp
        resource_manager/ResourceManager.hpp resource_manager/ResourcesLoaders.cpp resource_manager/ResourcesLoaders.hpp)

set(OPEN_DEMO_SRC_MODULE_WINDOWING
        windowing/Window.hpp
		windowing/Window.cpp
		windowing/InputtingWindow.hpp
		windowing/InputtingWindow.cpp
        windowing/Windowing.hpp
		windowing/Windowing.cpp
        windowing/WindowSettings.hpp)

set(OPEN_DEMO_SRC_MODULE_RENDER
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
        rendering/RenderContext.cpp
        rendering/RenderPipeline.cpp
        rendering/RenderPipeline.hpp
        rendering/BlendingDescription.hpp
        rendering/DepthDescription.hpp)

set(OPEN_DEMO_SRC_SCENES
        scenes/Scene_1.hpp
        scenes/Scene_1.cpp
        scenes/Scene_2.hpp
        scenes/Scene_2.cpp)

set(OPEN_DEMO_SRC
        ${OPEN_DEMO_SRC_DEPS_GLAD}
        ${OPEN_DEMO_SRC_COMMON}
        ${OPEN_DEMO_SRC_PLATFORM}
        ${OPEN_DEMO_SRC_MODULE_COMMON}
        ${OPEN_DEMO_SRC_MODULE_FILE_SYSTEM}
        ${OPEN_DEMO_SRC_MODULE_INPUTTING}
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
	set_target_properties(open_demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "/build/bin/")
    target_compile_options(open_demo PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(open_demo PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)