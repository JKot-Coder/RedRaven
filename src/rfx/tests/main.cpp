#include "rfx.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#define APPROVALS_CATCH
#include "ApprovalTests/ApprovalTests.hpp"

ASAN_DEFAULT_OPTIONS

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
            // config.testsOrTags.push_back("[CopyCommandList]");
            // config.testsOrTags.push_back("[ComputeCommandList]");
            session.useConfigData(config);
        }

        return session.run(argc, argv);
    }
}

int main(int argc, char** argv)
{
    return runApp(argc, argv);
}