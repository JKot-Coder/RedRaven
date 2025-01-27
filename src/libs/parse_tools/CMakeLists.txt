project (parse_tools)

set(SRC
    "DiagnosticCore.hpp"
    "DiagnosticDefinitions.hpp"
    "DiagnosticSink.cpp"
    "DiagnosticSink.hpp"
    "Lexer.cpp"
    "Lexer.hpp"
    "LexerDiagnosticDefinitions.hpp"
    "Preprocessor.cpp"
    "Preprocessor.hpp"
    "Token.cpp"
    "Token.hpp"
    "TokenDefinitions.hpp")
source_group( "" FILES ${SRC} )
set(CORE_SRC
    "core/CompileContext.hpp"
    "core/FileSystem.cpp"
    "core/FileSystem.hpp"
    "core/IncludeSystem.cpp"
    "core/IncludeSystem.hpp"
    "core/Signal.cpp"
    "core/Signal.hpp"
    "core/SourceLocation.hpp"
    "core/SourceManager.cpp"
    "core/SourceManager.hpp"
    "core/SourceView.hpp"
    "core/SourceView.cpp"
    "core/StringEscapeUtil.cpp"
    "core/StringEscapeUtil.hpp"
    "core/StringUtils.hpp"
    "core/UnownedStringSlice.cpp"
    "core/UnownedStringSlice.hpp")
source_group( "core" FILES ${CORE_SRC} )

add_library(${PROJECT_NAME} ${SRC} ${CORE_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
target_include_directories(${PROJECT_NAME} PRIVATE "..")
target_link_libraries(${PROJECT_NAME} common)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)