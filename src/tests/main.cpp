#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/internal/catch_debugger.hpp>
#include <catch2/internal/catch_compiler_capabilities.hpp>
#include <catch2/internal/catch_config_wchar.hpp>
#include <catch2/internal/catch_leak_detector.hpp>
#include <catch2/internal/catch_platform.hpp>

#include "common/debug/LeakDetector.hpp"

namespace Catch
{
    CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
    CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
    LeakDetector leakDetector;
    CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
}

int runCatch2(int argc, char** argv)
{
    const auto& leakDetector = OpenDemo::Common::Debug::LeakDetector::Instance();

    const auto& startSnapshot = leakDetector.CreateEmpySnapshot();
    const auto& finishSnapshot = leakDetector.CreateEmpySnapshot();

    int result = 0;

    leakDetector.Capture(startSnapshot);

    {
        // We want to force the linker not to discard the global variable
        // and its constructor, as it (optionally) registers leak detector
        (void)&Catch::leakDetector;

        auto& session = Catch::Session();

        if (Catch::isDebuggerActive())
        {
            Catch::ConfigData config;
            config.showDurations = Catch::ShowDurations::Always;
            config.useColour = Catch::UseColour::No;
            config.outputFilename = "%debug";
            session.useConfigData(config);
        }

        result = session.run(argc, argv);
    }

    leakDetector.Capture(finishSnapshot);

    if (leakDetector.GetDifference(startSnapshot, finishSnapshot))
    {
        leakDetector.DumpAllSince(startSnapshot);
        return (result == 0) ? -1 : result;
    }

    return result;
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

    const auto exitCode = runCatch2(argc, argv);

    for (int i = 0; i < argc; ++i)
        free(argv[i]);
    free(argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return runCatch2(argc, argv);
}
#endif

unsigned int Factorial(unsigned int number)
{
    return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]")
{
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);
}