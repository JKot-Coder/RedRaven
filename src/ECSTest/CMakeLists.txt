project(ECSTest)

set(SRC
    "pch.hpp"
    "main.cpp"
    "Application.hpp"
    "Application.cpp"
    "testES.inl"
    "testES.cpp"
)
source_group( "" FILES ${SRC} )

set(SRC
    ${SRC})

set(TESTS_LINK_LIBRARIES
    common
    flecs
    )

set(TESTS_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs
    ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)