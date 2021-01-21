#include "common/debug/LeakDetector.hpp"

#include "Application.hpp"

namespace
{
    int runApp(int argc, char** argv)
    {
        const auto& leakDetector = OpenDemo::Common::Debug::LeakDetector::Instance();
        auto application = std::make_unique<OpenDemo::Tests::Application>();

        return application->Run(argc, argv);
    }
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <Windows.h>
#include <shellapi.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow)
{
    int argc;
    LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char** argv = (char**)malloc(argc * sizeof(char*));

    for (int i = 0; i < argc; ++i)
    {
        const auto size = wcslen(lpArgv[i]) + 1;
        argv[i] = static_cast<char*>(malloc(size));
        wcstombs(argv[i], lpArgv[i], size);
    }
    LocalFree(lpArgv);

    const auto exitCode = runApp(argc, argv);

    for (int i = 0; i < argc; ++i)
        free(argv[i]);
    free(argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return runApp(argc, argv);
}
#endif