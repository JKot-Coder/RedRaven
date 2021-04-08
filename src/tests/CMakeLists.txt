project(tests)

set(SRC
    "pch.hpp"
    "main.cpp"
    "Application.hpp"
    "Application.cpp"
    "TestContextFixture.cpp"
    "TestContextFixture.hpp")
source_group( "" FILES ${SRC} )

set(TESTS_SRC
    "Tests/CopyCommandList.cpp"
    "Tests/CopyCommandList.hpp" 
)
source_group( "Tests" FILES ${TESTS_SRC} )

set(APPROVAL_INTEGRAION_SRC
    "ApprovalIntegration/ImageWriter.hpp"
    "ApprovalIntegration/ImageComparator.hpp"
    "ApprovalIntegration/ImageApprover.hpp"
    )
source_group( "ApprovalIntegration" FILES ${APPROVAL_INTEGRAION_SRC} )

set(SRC
    ${SRC}
    ${TESTS_SRC}
    ${APPROVAL_INTEGRAION_SRC})

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



add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)