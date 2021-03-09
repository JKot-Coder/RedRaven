project (windowing)

set( WINDOWING_SRC
       # InputtingWindow.hpp
      #  InputtingWindow.cpp
        Window.hpp
        Windowing.hpp
        Windowing.cpp )

if (WIN32)

set( WINDOWING_SRC
     ${WINDOWING_SRC}
     Win32Window.cpp
)
endif (WIN32)

source_group( "" FILES ${WINDOWING_SRC} )

add_library(${PROJECT_NAME} ${WINDOWING_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common)
