project(rfx_tests)

set(RFX_TESTS_SRC
    "main.cpp"
    "RfxApprover.hpp"
    "RfxWriter.cpp"
    "RfxWriter.hpp"
    "TestUtils.hpp"
    "TestUtils.cpp")
source_group( "" FILES ${RFX_TESTS_SRC} )

set(RFX_TESTS_LEXER_SRC
    "lexer/Lexer.cpp"
    "lexer/BasicTokens.rfx"
    "lexer/EofInBlockLine.rfx"
    "lexer/Multiline.rfx"
    "lexer/Numbers.rfx"
    "lexer/StringLiteras.rfx")
source_group( "lexer" FILES ${RFX_TESTS_LEXER_SRC} )

set(RFX_TESTS_COMPILE_SRC
    "command_line/CommandLine.cpp"  
    "command_line/help.rfx")
source_group( "commandLine" FILES ${RFX_TESTS_COMPILE_SRC} )

set(SRC
    ${RFX_TESTS_SRC}
    ${RFX_TESTS_LEXER_SRC}
    ${RFX_TESTS_COMPILE_SRC})

set(RFX_TESTS_LINK_LIBRARIES
    common
    rfx
    Catch2::Catch2WithMain
    ApprovalTests
    cpp-command-output)

add_subdirectory(cpp-command-output)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${SRC})
add_dependencies(${PROJECT_NAME} rfxc)
target_link_libraries(${PROJECT_NAME} ${RFX_TESTS_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "rfx")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

#=============================== Tests ================================#

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
