project(rfx_tests)

set(RFX_TESTS_SRC
    "main.cpp")
source_group( "" FILES ${RFX_TESTS_SRC} )

set(RFX_TESTS_LINK_LIBRARIES
    common
    rfx
    Catch2::Catch2WithMain
    ApprovalTests)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${RFX_TESTS_SRC})
target_link_libraries(${PROJECT_NAME} ${RFX_TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "rfx")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)