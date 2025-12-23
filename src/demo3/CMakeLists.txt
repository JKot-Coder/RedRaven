project(demo3)

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
target_add_shaders(${PROJECT_NAME} SHADERS ${SHADERS_SRC})
target_copy_webgpu_binaries(${PROJECT_NAME})