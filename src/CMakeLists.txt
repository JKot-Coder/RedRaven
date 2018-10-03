set(sources
       ./core.h
        input.h vecmath.h)
set(definitions)

set(GAPI AUTO CACHE STRING "Graphics api")
set_property(CACHE GAPI PROPERTY STRINGS OPENGL)

if (WIN32)
    list(APPEND sources
            platform/win/main.cpp
            )
    if(GAPI STREQUAL AUTO)
        SET(GAPI OPENGL)
    endif()
endif ()

if (LINUX)
    list(APPEND sources
            platform/linux/main.cpp
            )
    if(GAPI STREQUAL AUTO)
        SET(GAPI OPENGL)
    endif()
endif ()

if (GAPI STREQUAL OPENGL)
    list(APPEND sources
            gapi/gl.h
            )
endif ()

include_directories (. gapi)

add_executable (open_demo ${sources})

if (GAPI STREQUAL OPENGL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(open_demo ${OPENGL_LIBRARIES})
    list(APPEND definitions GAPI_GL)
endif ()

if (WIN32)
    list(APPEND definitions OS_WIN NOMINMAX)
endif ()

target_compile_definitions(open_demo PRIVATE ${definitions})