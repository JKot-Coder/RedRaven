project(demo2)

set(SRC
    Application.cpp
    Application.hpp
    main.cpp)
source_group("" FILES ${SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    ecs
    ecs_window
    render
    shader
)

set(SHADERS_SRC
    shader.jsonnet)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})

if (PLATFORM_WIN32)
    copy_required_dlls(demo2 DXC_REQUIRED YES)
endif()
target_add_shaders(demo2 SHADERS ${SHADERS_SRC})