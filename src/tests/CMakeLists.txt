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
    "Tests/ComputeCommandList.hpp"
    "Tests/ComputeCommandList.cpp"
)
source_group( "Tests" FILES ${TESTS_SRC} )

set(APPROVAL_INTEGRAION_SRC
    "ApprovalIntegration/BufferApprover.hpp"
    "ApprovalIntegration/BufferWriter.cpp"
    "ApprovalIntegration/BufferWriter.hpp"
    "ApprovalIntegration/ImageApprover.hpp"
    "ApprovalIntegration/ImageComparator.hpp"
    "ApprovalIntegration/ImageComporator.cpp"
    "ApprovalIntegration/ImageWriter.cpp"
    "ApprovalIntegration/ImageWriter.hpp"
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
    gapi_dx12
    platform
    Catch2::Catch2
    ApprovalTests
    DirectXTex
    )

set(TESTS_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs
    ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} WIN32 ${SRC})
target_link_libraries(${PROJECT_NAME} ${TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)