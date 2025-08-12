#include "Application.hpp"

int entryPoint(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);

    RR::App::Application app;
    return app.Run();
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <Windows.h>
#include <shellapi.h>
#include <wchar.h>
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

    const auto exitCode = entryPoint(argc, argv);

    for (int i = 0; i < argc; ++i)
        delete[] (argv[i]);
    delete[] (argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return entryPoint(argc, argv);
}
#endif
