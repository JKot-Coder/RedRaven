#include "Application.hpp"
#include "common/debug/LeakDetector.hpp"
#include "fmt/core.h"

namespace
{
    int runApp(int argc, char** argv)
    {
        const auto& leakDetector = RR::Common::Debug::LeakDetector::Instance();
        std::ignore = leakDetector;

        auto& application = RR::Tests::Application::Instance();

        return application.Run(argc, argv);
    }
}

int main(int argc, char** argv) { return runApp(argc, argv); }