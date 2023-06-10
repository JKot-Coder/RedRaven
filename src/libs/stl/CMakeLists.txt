project (stl)

set( SRC
    enum_array.hpp
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} INTERFACE)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} INTERFACE magic_enum)