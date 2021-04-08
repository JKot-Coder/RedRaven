project(tests)

set(TESTS_SRC
    "pch.hpp"
    "main.cpp"
    "Application.hpp"
    "Application.cpp"
    "TestContextFixture.cpp"
    "TestContextFixture.hpp"
    "Tests/CopyCommandList.cpp"
    "Tests/CopyCommandList.hpp"
    "ApprovalIntegration/ImageComparator.hpp")

set(TESTS_LINK_LIBRARIES
    common
    render
    gapi
    windowing
    Catch2::Catch2WithMain
    ApprovalTests)

set(TESTS_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs
    ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} WIN32 ${TESTS_SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)