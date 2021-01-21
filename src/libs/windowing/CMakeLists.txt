project (windowing)

set( WINDOWING_SRC
        InputtingWindow.hpp
        InputtingWindow.cpp
        Window.hpp
        Window.cpp
        Windowing.hpp
        Windowing.cpp
        WindowSettings.hpp )

source_group( "" FILES ${GAPI_SRC} )

add_library(${PROJECT_NAME} ${WINDOWING_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} common CONAN_PKG::sdl2)
target_include_directories(${PROJECT_NAME} PRIVATE "..")
