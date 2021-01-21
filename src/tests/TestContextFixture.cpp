#include "TestContextFixture.hpp"

#include "windowing/InputtingWindow.hpp"
#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"

#include "render/RenderContext.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        TestContextFixture::TestContextFixture()
        {
        }

        TestContextFixture::~TestContextFixture()
        {
            auto& renderContext = Render::RenderContext::Instance();
            renderContext.Terminate();
        }

        bool TestContextFixture::Init()
        {
            Windowing::WindowSettings settings;

            Windowing::WindowRect rect(Windowing::WindowRect::WINDOW_POSITION_CENTERED,
            Windowing::WindowRect::WINDOW_POSITION_CENTERED, 640, 480);

            settings.Title = "Tests";
            settings.WindowRect = rect;

            window_ = std::shared_ptr<Windowing::InputtingWindow>(new Windowing::InputtingWindow());
            if (!window_->Init(settings, true))
            {
                ASSERT_MSG(false, "Error initing window");
                return false;
            }

            auto& renderContext = Render::RenderContext::Instance();
            if (!renderContext.Init())
            {
                ASSERT_MSG(false, "Error initialize render context");
                return false;
            }

            return true;
        }

    }
}