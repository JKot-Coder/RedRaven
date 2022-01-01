#include "experement.hpp"

#include <slang.h>

#include "core/FileSystem.hpp"
#include "core/IncludeSystem.hpp"

#include "compiler/DiagnosticCore.hpp"
#include "compiler/Preprocessor.hpp"
#include "compiler/SourceLocation.hpp"
#include "compiler/Token.hpp"

#include <fstream>

void test2()
{
    std::ifstream instream("test2.slang");
    RR::U8String input(
        std::istreambuf_iterator<char>(instream.rdbuf()),
        std::istreambuf_iterator<char>());

    const auto& pathInfo = RR::Rfx::Compiler::PathInfo::makeNormal("test2.slang", "test2.slang");
    const auto& sourceFile = std::make_shared<RR::Rfx::Compiler::SourceFile>(pathInfo); //, SourceRange range, const U8String* viewPath, SourceLocation initiatingSourceLocation)
    sourceFile->SetContents(input);

    const auto& fileSystem = std::make_shared<RR::Rfx::Compiler::OSFileSystem>();
    const auto& includeSystem = std::make_shared<RR::Rfx::Compiler::IncludeSystem>(fileSystem);
    const auto& diagnosticSink = std::make_shared<RR::Rfx::Compiler::DiagnosticSink>();
    const auto& preprocessor = std::make_shared<RR::Rfx::Compiler::Preprocessor>(sourceFile, diagnosticSink, includeSystem);
    const auto& tokens = preprocessor->ReadAllTokens();

    for (auto token : tokens)
    {
        Log::Format::Info("Token:{{Type:\"{0}\", Content:\"{1}\", Line:{2} Column:{3}}}\n", RR::Rfx::Compiler::TokenTypeToString(token.type), token.GetContentString(), token.humaneSourceLocation.line, token.humaneSourceLocation.column);

        if (token.type == RR::Rfx::Compiler::TokenType::EndOfFile)
            break;
    }
}
/*
void test()
{
    ::Slang::ComPtr<slang::IGlobalSession> globalSesion;
    slang::createGlobalSession(globalSesion.writeRef());

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_DXBC_ASM;
    targetDesc.profile = globalSesion->findProfile("sm_5_0");
    targetDesc.optimizationLevel = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
    targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
    targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
    targetDesc.flags = 0;

    ::Slang::ComPtr<slang::ISession> session;
    slang::SessionDesc desc = {};
    desc.targets = &targetDesc;
    desc.targetCount = 1;
    globalSesion->createSession(desc, session.writeRef());

    ::Slang::ComPtr<slang::ICompileRequest> request;
    session->createCompileRequest(request.writeRef());

    //request->setCodeGenTarget(SLANG_TARGET_NONE);
    // request->setOutputContainerFormat(SLANG_CONTAINER_FORMAT_SLANG_MODULE);

    Slang::ComPtr<slang::IBlob> codeBlob;

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "");
    spAddTranslationUnitSourceFile(request, translationUnitIndex, "test.slang");

    auto entryPointIndex = request->addEntryPoint(translationUnitIndex, "computeMain", SLANG_STAGE_COMPUTE);
    ::Slang::ComPtr<slang::IComponentType> entryPoint;

    if (SLANG_FAILED(request->compile()))
    {
        char const* diagnostics = request->getDiagnosticOutput();
        Log::Print::Warning(diagnostics);
        request.setNull();
        session.setNull();
    }

    //   int entryPointIndex = 0; // only one entry point
    //  int targetIndex = 0; // only one target
    ::Slang::ComPtr<slang::IBlob> kernelBlob;
    ::Slang::ComPtr<slang::IBlob> diagnostics;

    const char* casda = (const char*)request->getEntryPointSource(0);
    Log::Print::Warning(casda);

    request->getContainerCode(codeBlob.writeRef());

    if (codeBlob)
    {
        Log::Print::Warning((const char*)codeBlob->getBufferPointer());
    }

    (void)entryPointIndex;
}
 */