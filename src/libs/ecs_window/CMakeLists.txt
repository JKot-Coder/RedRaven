project (ecs_window)

set(SRC
    Window.hpp
    Window.cpp)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} RR::BuildSettings ecs glfw)

add_library("${PROJECT_NAME}_interface" INTERFACE)
target_include_directories("${PROJECT_NAME}_interface" INTERFACE "..")
add_library("${PROJECT_NAME}::interface" ALIAS "${PROJECT_NAME}_interface")