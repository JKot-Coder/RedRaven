project(rfx)

set(RFX_SRC
    "pch.hpp"
    "experement.hpp"
    "experement.cpp"
    "include/rfx.hpp"
    "main.cpp")
source_group( "" FILES ${RFX_SRC} )

set(RFX_CORE_SRC
    "core/Exception.hpp"
    "core/FileSystem.cpp"
    "core/FileSystem.hpp"
    "core/IncludeSystem.cpp"
    "core/IncludeSystem.hpp"
    "core/SourceLocation.cpp"
    "core/SourceLocation.hpp"
    "core/UnownedStringSlice.cpp"
    "core/UnownedStringSlice.hpp"
    )
source_group( "core" FILES ${RFX_CORE_SRC} )

set(RFX_COMPILER_SRC
    "compiler/CompileRequest.cpp"
    "compiler/CompileRequest.hpp"
    "compiler/DiagnosticCore.cpp"
    "compiler/DiagnosticCore.hpp"
    "compiler/DiagnosticDefinitions.hpp"
    "compiler/DiagnosticSink.cpp"
    "compiler/DiagnosticSink.hpp"
    "compiler/Effect.cpp"
    "compiler/Effect.hpp"
    "compiler/InputString.hpp"
    "compiler/Lexer.cpp"
    "compiler/Lexer.hpp"
    "compiler/LexerDiagnosticDefinitions.hpp"
    "compiler/Preprocessor.cpp"   
    "compiler/Preprocessor.hpp"
    "compiler/Program.cpp"
    "compiler/Program.hpp"
    "compiler/Session.cpp"
    "compiler/Session.hpp"
    "compiler/Signal.cpp"
    "compiler/Signal.hpp"
    "compiler/Token.cpp"
    "compiler/Token.hpp"
    "compiler/TokenDefinitions.hpp"
    )
source_group( "compiler" FILES ${RFX_COMPILER_SRC} )


set(RFX_SRC
    ${RFX_SRC}
    ${RFX_CORE_SRC}
    ${RFX_COMPILER_SRC})

set(RFX_LINK_LIBRARIES
    common
    slang
    nlohmann_json::nlohmann_json)

set(RFX_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/libs
    ${PROJECT_SOURCE_DIR})

#================================ Slang ===============================#

FetchContent_Declare(slang_download
  URL https://github.com/shader-slang/slang/releases/download/v0.18.24/slang-0.18.24-win64.zip
  URL_HASH "SHA512=3836940f0877a4bb8144bfb83fd921e67b57ea8f8912245fcf0d9cdfd5485774135fa5d3689b1717cec88ec95691c0a618b1e45f00da75635904a701d25f629c"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  )
FetchContent_MakeAvailable(slang_download)

file(COPY ${slang_download_SOURCE_DIR}/bin/windows-x64/release/slang.dll
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_library(slang STATIC IMPORTED)
target_include_directories(slang INTERFACE ${slang_download_SOURCE_DIR}/)
set_target_properties(slang PROPERTIES
    IMPORTED_LOCATION ${slang_download_SOURCE_DIR}/bin/windows-x64/release/slang.lib
)

add_dependencies(slang slang_download)

#============================ FetchContent ============================#

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.9.1)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install  OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(json)

#=============================== Target ===============================#

add_executable(${PROJECT_NAME} WIN32 ${RFX_SRC})
target_link_libraries(${PROJECT_NAME} ${RFX_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${RFX_INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)