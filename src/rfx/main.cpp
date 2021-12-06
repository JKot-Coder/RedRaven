#include "compiler/CompileRequest.hpp"
#include "compiler/Program.hpp"
#include "compiler/Session.hpp"
#include "experement.hpp"

#ifdef OS_WINDOWS
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

#else
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
#endif

    test2();
    /*
    auto session = new Rfx::Compiler::Session();

    auto request = session->CreateCompileRequest();

    std::string log;

    const auto loaded = request->LoadModule("test.slang", log);
    if (!loaded)
    {
        Log::Print::Warning(log);
        return -1;
    }

    const auto entryPoint = request->AddEntryPoint();
    if (!entryPoint)
    {
        //Log::Print::Warning(log);
        return -1;
    }

    const auto program = request->Compile(log);

    if (!program)
    {
        Log::Print::Warning(log);
        return -1;
    }

    auto shaderProgram = program->GetShaderProgram(log);
    if (!shaderProgram)
    {
        Log::Print::Warning(log);
        return -1;
    }*/

    /*
    SlangSession* session = spCreateSession(NULL);

    SlangCompileRequest* request = spCreateCompileRequest(session);
    spSetCodeGenTarget(request, SLANG_SPIRV);

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_HLSL, "");
    spAddTranslationUnitSourceFile(request, translationUnitIndex, "some/file.hlsl");
    SlangProfileID profileID = spFindProfile(session, "ps_5_0");
    int entryPointIndex = spAddEntryPoint(
        request,
        translationUnitIndex,
        "main",
        profileID);

    int anyErrors = spCompile(request);

    if (anyErrors != 0)
    {
        char const* diagnostics = spGetDiagnosticOutput(request);
        Log::Print::Warning(diagnostics);
    }

    spDestroyCompileRequest(request);
    spDestroySession(session);*/

    return 0;
}