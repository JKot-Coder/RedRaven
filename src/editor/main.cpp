#include "Application.hpp"

namespace
{
    int runApp(int argc, char** argv)
    {

        std::ignore = argc;
        std::ignore = argv;

        RR::Application application;
        return application.Run();
    }
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <WCHAR.h>
#include <Windows.h>
#include <shellapi.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow)
{
    std::ignore = hInst;
    std::ignore = hPreInst;
    std::ignore = lpCmdLine;
    std::ignore = nCmdShow;

    int argc;
    wchar_t** lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i)
    {
        const auto size = wcslen(lpArgv[i]) + 1;
        argv[i] = new char[size];
        size_t countConverted;

        wcsrtombs_s(
            &countConverted,
            argv[i],
            size,
            const_cast<const wchar_t**>(&lpArgv[i]),
            size - 1,
            nullptr);
    }
    LocalFree(lpArgv);

    const auto exitCode = runApp(argc, argv);

    for (int i = 0; i < argc; ++i)
        delete[](argv[i]);
    delete[](argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return runApp(argc, argv);
}
#endif