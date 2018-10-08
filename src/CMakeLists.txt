set(sources
       ./core.h
        input.h
        vecmath.h
        render/render.h)

set(definitions)

set(render AUTO CACHE STRING "Graphics api")
set_property(CACHE render PROPERTY STRINGS OPENGL)

if (WIN32)
    list(APPEND sources
            platform/win/main.cpp
            )
    if(render STREQUAL AUTO)
        SET(render OPENGL)
    endif()
endif ()

if (LINUX)
    list(APPEND sources
            platform/linux/main.cpp
            )
    if(render STREQUAL AUTO)
        SET(render OPENGL)
    endif()
endif ()

if (render STREQUAL OPENGL)
    list(APPEND sources
            render/render_gl.h
            )
endif ()

include_directories (. render)

add_executable (open_demo ${sources})

if (render STREQUAL OPENGL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(open_demo ${OPENGL_LIBRARIES})
    list(APPEND definitions RENDER_GL)
endif ()

if (WIN32)
    list(APPEND definitions OS_WIN NOMINMAX)
endif ()

target_compile_definitions(open_demo PRIVATE ${definitions})