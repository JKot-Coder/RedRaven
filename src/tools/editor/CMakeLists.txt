project(editor)

set(SRC
    "Application.cpp"
    "Application.hpp"
    "main.cpp")
source_group("" FILES ${SRC})

set(IMGUI_IMPL_SRC
    "imgui_impl/ImguiPlatformImpl.cpp"
    "imgui_impl/ImguiPlatformImpl.hpp")
source_group("ImguiImpl" FILES ${IMGUI_IMPL_SRC})

set(SRC
    ${SRC}
    ${IMGUI_IMPL_SRC})

set(LIBRARIES
    RR::BuildSettings
    common
    platform
    render
    imgui::imgui
    ecs_module
    ecs_imgui
    )

if (WIN32)
    set(EDITOR_LINK_LIBRARIES ${EDITOR_LINK_LIBRARIES} gapi_dx12)
endif()

add_subdirectory(editor_ecs)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
add_dependencies(${PROJECT_NAME} editor_ecs)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")