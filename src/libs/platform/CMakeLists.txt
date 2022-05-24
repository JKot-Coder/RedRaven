project (platform)

set(SRC
    "GlfwWindowImpl.cpp"
    "GlfwWindowImpl.hpp"
    "GlfwWindowSystem.cpp"
    "Input.cpp"
    "Input.hpp"
    "Platform.cpp"
    "Platform.hpp"
    "Window.hpp"
    "WindowSystem.hpp"
    )

source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common glfw)