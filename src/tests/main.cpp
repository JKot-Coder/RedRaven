#include <catch2/catch_all.hpp>
    /*
#include <catch2/internal/catch_default_main.hpp>


isDebuggerActive



    Catch::ConfigData config;
    config.showDurations = Catch::ShowDurations::Always;
    config.useColour = Catch::UseColour::No;
    config.outputFilename = "%debug";
    session.useConfigData( config );



    return session.run( argc, argv );



namespace Catch
{
    CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
    CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
    LeakDetector leakDetector;
    CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
}

int executTests(int argc, char** argv)
{
    // We want to force the linker not to discard the global variable
    // and its constructor, as it (optionally) registers leak detector
    (void)&Catch::leakDetector;

    return Catch::Session().run(argc, argv);
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <Windows.h>
#include <shellapi.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow)
{
    int argc;
    char** argv;

    LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    argv = (char**)malloc(argc * sizeof(char*));
    for (int i = 0; i < argc; ++i)
    {
        const auto size = wcslen(lpArgv[i]) + 1;
        argv[i] = static_cast<char*>(malloc(size));
        wcstombs(argv[i], lpArgv[i], size);
    }
    LocalFree(lpArgv);

    const auto exitCode = executTests(argc, argv);

    for (int i = 0; i < argc; ++i)
        free(argv[i]);
    free(argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return executTests(argc, argv);
}
#endif
*/
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