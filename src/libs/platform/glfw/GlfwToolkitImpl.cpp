#include "platform/Toolkit.hpp"

#include "platform/Window.hpp"
#include "platform/glfw/GlfwWindowImpl.hpp"

namespace RR
{
    namespace Platform
    {
        Toolkit::~Toolkit()
        {
            glfwTerminate();
        }

        void Toolkit::Init()
        {
            ASSERT(!isInited_);

            if (!glfwInit())
                LOG_FATAL("Can't initializate glfw");

            isInited_ = true;
        }

        std::vector<Monitor> Toolkit::GetMonitors() const
        {
            ASSERT(isInited_);

            std::vector<Monitor> result;

            int monitorsCount = 0;
            GLFWmonitor** glfwMonitors = glfwGetMonitors(&monitorsCount);

            for (int index = 0; index < monitorsCount; index++)
            {
                Monitor& monitor = result.emplace_back();

                auto videoMode = glfwGetVideoMode(glfwMonitors[index]);
                monitor.videoMode.resolution = Vector2i(videoMode->width, videoMode->height);
                monitor.videoMode.refreshRate = videoMode->refreshRate;
                monitor.videoMode.redBits = videoMode->redBits;
                monitor.videoMode.greenBits = videoMode->greenBits;
                monitor.videoMode.blueBits = videoMode->blueBits;

                glfwGetMonitorPos(glfwMonitors[index], &monitor.position.x, &monitor.position.y);
                glfwGetMonitorWorkarea(glfwMonitors[index], &monitor.workArea.left, &monitor.workArea.top, &monitor.workArea.width, &monitor.workArea.height);
                // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
                glfwGetMonitorContentScale(glfwMonitors[index], &monitor.dpiScale.x, &monitor.dpiScale.y);
            }
            //   glfwSetMonitorCallback()

            return result;
        }

        std::shared_ptr<Window> Toolkit::CreatePlatformWindow(const Window::Description& description) const
        {
            ASSERT(isInited_);

            return GlfwWindowImpl::Create(description);
        }

        void Toolkit::PoolEvents() const
        {
            ASSERT(isInited_);

            glfwPollEvents();
        }
    }
}