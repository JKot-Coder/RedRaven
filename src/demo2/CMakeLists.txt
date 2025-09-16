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

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})

copy_required_dlls(demo2 DXC_REQUIRED YES)
target_add_shaders(demo2 SHADERS)