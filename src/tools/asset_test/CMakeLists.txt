project(AssetTest)

set(SRC
    "main.cpp"
    "Application.hpp"
    "Application.cpp"
)
source_group( "" FILES ${SRC} )

set(SRC
    ${SRC})

set(TESTS_LINK_LIBRARIES
    RR::BuildSettings
    common
    asset_server
    )

set(TESTS_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs
    ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tools")