project(editor_ecs)

set(SRC
    "main.cpp")
source_group("" FILES ${SRC})

set(SRC ${SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    imgui::imgui
    ecs
    ecs_module::interface
    cr)

#=============================== Target ===============================#

add_library(${PROJECT_NAME} SHARED ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")