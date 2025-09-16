project(editor_ecs)

set(SRC
    main.cpp
    test.cpp
    InitImguiContext.cpp)
source_group("" FILES ${SRC})

set(SRC ${SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    imgui::imgui
    ecs
    ecs_module::interface
    ecs_imgui::interface
    cr)

#=============================== Target ===============================#

add_library(${PROJECT_NAME} SHARED ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")