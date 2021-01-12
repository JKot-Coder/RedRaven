project(tests)

set(TESTS_SRC
    "main.cpp")

set(TESTS_LINK_LIBRARIES
    common
    boost.ut)

set(TESTS_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs)

add_executable(${PROJECT_NAME} WIN32 ${TESTS_SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)