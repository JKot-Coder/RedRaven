#include "Application.hpp"

#include "gapi/CommandQueue.hpp"
#include "platform/Toolkit.hpp"
#include "render/DeviceContext.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#define APPROVALS_CATCH
#include "ApprovalTests/ApprovalTests.hpp"

#include "ApprovalIntegration/ImageComparator.hpp"

namespace RR
{
    namespace Tests
    {
        int Application::Run(int argc, char** argv)
        {
            if (!init())
                return -1;

            int result = 0;
            {
                // Custom comporator for images.
                auto ktxComparatorDisposer =
                    ApprovalTests::FileApprover::registerComparatorForExtension(
                        ".dds", std::make_shared<ImageComparator>());

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
                    //       config.testsOrTags.push_back("[CopyCommandList]");
                    //     config.testsOrTags.push_back("[ComputeCommandList]");
                    session.useConfigData(config);
                }

                result = session.run(argc, argv);
            }

            terminate();

            return result;
        }

        bool Application::init()
        {
            Platform::Window::Description description;
            description.size = Vector2i(640, 480);
            description.title = "Tests";
            /*
            window_ = Windowing std::shared_ptr<Platform::InputtingWindow>(new Platform::InputtingWindow());
            if (!window_->Init(description, true))
            {
                ASSERT_MSG(false, "Error initing window");
                return false;
            }*/

            auto& deviceContext = Render::DeviceContext::Instance();
            deviceContext.Init();
            /*
            TODO

            if (!deviceContext.Init())
            {
                ASSERT_MSG(false, "Error initialize render context");
                return false;
            }
            */

            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Copy)] = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "Copy");
            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Compute)] = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Compute, "Compute");
            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Graphics)] = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Graphics, "Primary");

            return true;
        }

        void Application::terminate()
        {
            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Copy)] = nullptr;
            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Compute)] = nullptr;
            commandQueue_[static_cast<size_t>(GAPI::CommandQueueType::Graphics)] = nullptr;

            auto& deviceContext = Render::DeviceContext::Instance();
            deviceContext.Terminate();
        }
    }
}