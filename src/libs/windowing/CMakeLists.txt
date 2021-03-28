project (windowing)

set( WINDOWING_SRC
       # InputtingWindow.hpp
      #  InputtingWindow.cpp
        Window.hpp
        Window.cpp
        WindowSystem.hpp
        GlfwWindowImpl.hpp
        GlfwWindowImpl.cpp
        GlfwWindowSystem.cpp
     )

source_group( "" FILES ${WINDOWING_SRC} )

add_library(${PROJECT_NAME} ${WINDOWING_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common glfw)