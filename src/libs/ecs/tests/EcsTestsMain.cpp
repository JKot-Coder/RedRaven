#define CATCH_CONFIG_RUNNER
#include <catch2/internal/catch_compiler_capabilities.hpp>
#include <catch2/internal/catch_leak_detector.hpp>
#include <catch2/catch_session.hpp>

ASAN_DEFAULT_OPTIONS

namespace Catch {
    CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
    CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
    static LeakDetector leakDetector;
    CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
}

namespace
{
    int runApp(int argc, char** argv)
    {
        // We want to force the linker not to discard the global variable
        // and its constructor, as it (optionally) registers leak detector
        (void)&Catch::leakDetector;

        auto session = Catch::Session();
        /*
        if (Catch::isDebuggerActive())
        {
            Catch::ConfigData config;
            config.showDurations = Catch::ShowDurations::Always;
            config.useColour = Catch::UseColour::No;
            config.outputFilename = "%debug";
            // config.testsOrTags.push_back("[CopyCommandList]");
            // config.testsOrTags.push_back("[ComputeCommandList]");
            session.useConfigData(config);
        }*/

        return session.run(argc, argv);
    }
}

int main(int argc, char** argv)
{
    return runApp(argc, argv);
}