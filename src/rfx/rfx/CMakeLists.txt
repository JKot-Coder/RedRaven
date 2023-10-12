project(rfx)

set(RFX_SRC
    "pch.hpp"
    "rfxApi.cpp")
source_group( "" FILES ${RFX_SRC} )

set(RFX_INCLUDE
    "include/rfx.hpp")
source_group( "include" FILES ${RFX_INCLUDE} )

set(RFX_CORE_SRC
    "core/Blob.hpp"
    "core/CStringAllocator.hpp"
    "core/Exception.hpp"
    "core/FileSystem.cpp"
    "core/FileSystem.hpp"
    "core/IncludeSystem.cpp"
    "core/IncludeSystem.hpp"
    "core/SourceLocation.hpp"
    "core/SourceView.hpp"
    "core/SourceView.cpp"
    "core/SourceManager.cpp"
    "core/SourceManager.hpp"
    "core/StringEscapeUtil.cpp"
    "core/StringEscapeUtil.hpp"
    "core/UnownedStringSlice.cpp"
    "core/UnownedStringSlice.hpp")
source_group( "core" FILES ${RFX_CORE_SRC} )

set(RFX_COMPILER_SRC
    "compiler/CompileContext.hpp"
    "compiler/CompileRequest.cpp"
    "compiler/CompileRequest.hpp"
    "compiler/DiagnosticCore.hpp"
    "compiler/DiagnosticDefinitions.hpp"
    "compiler/DiagnosticSink.cpp"
    "compiler/DiagnosticSink.hpp"
    "compiler/DxcPreprocessor.cpp"
    "compiler/DxcPreprocessor.hpp"
    "compiler/EffectCompiler.cpp"
    "compiler/EffectCompiler.hpp"
    "compiler/Lexer.cpp"
    "compiler/Lexer.hpp"
    "compiler/LexerDiagnosticDefinitions.hpp"
    "compiler/Parser.cpp"
    "compiler/Parser.hpp"
    "compiler/Preprocessor.cpp"
    "compiler/Preprocessor.hpp"
    "compiler/RSONBuilder.cpp"
    "compiler/RSONBuilder.hpp"
    "compiler/RSONValue.hpp"
    "compiler/RSONValue.cpp"
    "compiler/Signal.cpp"
    "compiler/Signal.hpp"
    "compiler/Token.cpp"
    "compiler/Token.hpp"
    "compiler/TokenDefinitions.hpp")
source_group( "compiler" FILES ${RFX_COMPILER_SRC} )

set(RFX_SRC
    ${RFX_SRC}
    ${RFX_INCLUDE}
    ${RFX_CORE_SRC}
    ${RFX_COMPILER_SRC})

add_subdirectory(dxc)

set(RFX_LINK_LIBRARIES
    common dxclib yaml-cpp)

option(BUILD_RFX_SHARED "Build rfx as a shared library" ON)

#================================== Target ===================================#

if(BUILD_RFX_SHARED)
    add_library(${PROJECT_NAME} SHARED ${RFX_SRC})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RFX_DYNAMIC_EXPORT)
    target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/..")
else()
    add_library(${PROJECT_NAME} ${RFX_SRC})
    target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/..")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} ${RFX_LINK_LIBRARIES})
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "rfx")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()