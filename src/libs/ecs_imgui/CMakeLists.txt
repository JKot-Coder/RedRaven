project (ecs_imgui)

set(SRC
    ImGui.cpp
    ImGui.hpp)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} ecs)

add_library("${PROJECT_NAME}_interface" INTERFACE)
target_include_directories("${PROJECT_NAME}_interface" INTERFACE "..")
add_library("${PROJECT_NAME}::interface" ALIAS "${PROJECT_NAME}_interface")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)