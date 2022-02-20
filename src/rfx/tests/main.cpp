#include "rfx/include/rfx.hpp"

#include "common/debug/LeakDetector.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#define APPROVALS_CATCH
#include "ApprovalTests/ApprovalTests.hpp"

namespace
{
    int runApp(int argc, char** argv)
    {
        std::ignore = argc;
        std::ignore = argv;

        // We want to force the linker not to discard the global variable
        // and its constructor, as it (optionally) registers leak detector
        (void)&Catch::leakDetector;

        auto session = Catch::Session();

        if (Catch::isDebuggerActive())
        {
            Catch::ConfigData config;
            config.showDurations = Catch::ShowDurations::Always;
            config.useColour = Catch::UseColour::No;
            config.outputFilename = "%debug";
            //    config.testsOrTags.push_back("[CopyCommandList]");
            // config.testsOrTags.push_back("[ComputeCommandList]");
            session.useConfigData(config);
        }

        return session.run(argc, argv);
    }
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <Windows.h>
#include <shellapi.h>
#include <WCHAR.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow)
{
    std::ignore = hInst;
    std::ignore = hPreInst;
    std::ignore = lpCmdLine;
    std::ignore = nCmdShow;

    const auto& leakDetector = RR::Common::Debug::LeakDetector::Instance();
    std::ignore = leakDetector;

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