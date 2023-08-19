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

set(RFX_TESTS_COMMAND_LINE_SRC
    "command_line/CommandLine.cpp"
    "command_line/compile_simple.rfx"
    "command_line/preprocessor_defines.rfx"
    "command_line/preprocessor_simple.rfx"
    "command_line/version.rfx"
    "command_line/help.rfx")
source_group( "commandLine" FILES ${RFX_TESTS_COMMAND_LINE_SRC} )

set(RFX_TESTS_PREPROCESSOR_SRC
    "preprocessor/SpecialMacroComplex.rfx"
    "preprocessor/bugs/Bug.rfx"
    "preprocessor/CommandLineDefines.rfx"
    "preprocessor/ConditionalDiagnostics.rfx"
    "preprocessor/DefineFunctionLike.rfx"
    "preprocessor/DefineRedefine.rfx"
    "preprocessor/DefineSimple.rfx"
    "preprocessor/DirectiveWhitespaces.rfx"
    "preprocessor/Error.rfx"
    "preprocessor/EscapedNewlines.rfx"
    "preprocessor/file-indentity/b.hrfx"
    "preprocessor/file-indentity/c.hrfx"
    "preprocessor/file-indentity/sub-folder/FileIdentity.rfx"
    "preprocessor/If.rfx"
    "preprocessor/Ifdef.rfx"
    "preprocessor/IfIgnore.rfx"
    "preprocessor/IfMacroTokenPaste.rfx"
    "preprocessor/Include.rfx"
    "preprocessor/include/PragmaOnce-c.hrfx"
    "preprocessor/include/SpecialMacroInclude.hrfx"
    "preprocessor/Include-a.hrfx"
    "preprocessor/IncludeMultiple.rfx"
    "preprocessor/macros/MacroParensFromExpansion.rfx"
    "preprocessor/macros/VariadicMacro.rfx"
    "preprocessor/PasteNonExpansion.rfx"
    "preprocessor/PoundPound_1.rfx"
    "preprocessor/PoundPound_2.rfx"
    "preprocessor/PragmaOnce.rfx"
    "preprocessor/PragmaOnce-a.hrfx"
    "preprocessor/PragmaOnce-b.hrfx"
    "preprocessor/PreprocConcat_1.rfx"
    "preprocessor/PreprocConcat_2.rfx"
    "preprocessor/PreprocConcat_3.rfx"
    "preprocessor/PreprocConcat_4.rfx"
    "preprocessor/PreprocDetail_1.rfx"
    "preprocessor/PreprocDetail_2.rfx"
    "preprocessor/PreprocDetail_3.rfx"
    "preprocessor/Preprocessor.cpp"
    "preprocessor/PreprocStringify.rfx"
    "preprocessor/RecursiveMacro.rfx"
    "preprocessor/RepeatedMacroExpansion.rfx"
    "preprocessor/SpecialMacroInclude.rfx"
    "preprocessor/SpecialMacroMultiLine.rfx"
    "preprocessor/SpecialMacroRedefineFunction.rfx"
    "preprocessor/SpecialMacroRedefineObject.rfx"
    "preprocessor/SpecialMacroSimple.rfx"
    "preprocessor/Undef.rfx"
    "preprocessor/Warning.rfx"
    "preprocessor/X-macro.rfx"
    )
source_group( "preprocessor" FILES ${RFX_TESTS_PREPROCESSOR_SRC} )

set(RFX_TESTS_PARSER_SRC
    "parser/Parser.cpp"
    "parser/Simple.rfx")
source_group( "parser" FILES ${RFX_TESTS_PARSER_SRC} )

set(SRC
    ${RFX_TESTS_SRC}
    ${RFX_TESTS_LEXER_SRC}
    ${RFX_TESTS_COMMAND_LINE_SRC}
    ${RFX_TESTS_PREPROCESSOR_SRC}
    ${RFX_TESTS_PARSER_SRC})

set(RFX_TESTS_LINK_LIBRARIES
    common
    rfx
    Catch2::Catch2
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

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
