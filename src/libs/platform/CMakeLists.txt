project (platform)

set(SRC
    "Input.cpp"
    "Input.hpp"
    "Window.hpp"
    "Toolkit.hpp"
    )
source_group( "" FILES ${SRC} )

set (GLFW_SRC
    "glfw/GlfwWindowImpl.cpp"
    "glfw/GlfwWindowImpl.hpp"
    "glfw/GlfwToolkitImpl.cpp"
    )
source_group( "glfw" FILES ${GLFW_SRC} )

set (SRC
    ${SRC}
    ${GLFW_SRC}
    )

add_library(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common glfw)