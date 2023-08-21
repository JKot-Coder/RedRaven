project (stl)

set( SRC
    stub.cpp
    enum_array.hpp
    enum.hpp
)
source_group( "" FILES ${SRC} )

add_library(${PROJECT_NAME} STATIC ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum EASTL)