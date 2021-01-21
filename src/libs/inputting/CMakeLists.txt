project (inputting)

set( INPUTTING_SRC
        Input.hpp
        Input.cpp)

source_group( "" FILES ${INPUTTING_SRC} )

add_library(${PROJECT_NAME} ${INPUTTING_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_link_libraries(${PROJECT_NAME} common)
target_include_directories(${PROJECT_NAME} PRIVATE "..")