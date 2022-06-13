#include "GlfwToolkit.hpp"

#include "platform/glfw/GlfwWindow.hpp"
#include <GLFW/glfw3.h>

namespace RR
{
    namespace Platform
    {
        namespace
        {
            void glfwMotiorToMonitor(GLFWmonitor* glfwMonitor, Monitor& monitor)
            {
                ASSERT(glfwMonitor);

                auto videoMode = glfwGetVideoMode(glfwMonitor);
                monitor.videoMode.resolution = Vector2i(videoMode->width, videoMode->height);
                monitor.videoMode.refreshRate = videoMode->refreshRate;
                monitor.videoMode.redBits = videoMode->redBits;
                monitor.videoMode.greenBits = videoMode->greenBits;
                monitor.videoMode.blueBits = videoMode->blueBits;

                glfwGetMonitorPos(glfwMonitor, &monitor.position.x, &monitor.position.y);
                glfwGetMonitorWorkarea(glfwMonitor, &monitor.workArea.left, &monitor.workArea.top, &monitor.workArea.width, &monitor.workArea.height);
                // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
                glfwGetMonitorContentScale(glfwMonitor, &monitor.dpiScale.x, &monitor.dpiScale.y);
            }

            void monitorCallback(GLFWmonitor* glfwMonitor, int glfwEvent)
            {
                Monitor::Event event;

                switch (glfwEvent)
                {
                    case GLFW_CONNECTED: event = Monitor::Event::Connected; break;
                    case GLFW_DISCONNECTED: event = Monitor::Event::Disconnected; break;
                    default: return;
                }

                Monitor monitor;
                glfwMotiorToMonitor(glfwMonitor, monitor);

                Toolkit::Instance().OnMonitorConfigChanged.Dispatch(monitor, event);
            }
        }

        GlfwToolkit::~GlfwToolkit()
        {
            glfwSetMonitorCallback(nullptr);

            glfwTerminate();
        }

        void GlfwToolkit::Init()
        {
            ASSERT(!isInited_);

            if (!glfwInit())
                LOG_FATAL("Can't initializate glfw");

            glfwSetMonitorCallback(&monitorCallback);

            isInited_ = true;
        }

        std::vector<Monitor> GlfwToolkit::GetMonitors() const
        {
            ASSERT(isInited_);

            std::vector<Monitor> result;

            int monitorsCount = 0;
            GLFWmonitor** glfwMonitors = glfwGetMonitors(&monitorsCount);

            for (int index = 0; index < monitorsCount; index++)
            {
                Monitor& monitor = result.emplace_back();
                glfwMotiorToMonitor(glfwMonitors[index], monitor);
            }

            return result;
        }

        std::shared_ptr<Window> GlfwToolkit::CreatePlatformWindow(const Window::Description& description) const
        {
            ASSERT(isInited_);

            return GlfwWindowImpl::Create(description);
        }

        void GlfwToolkit::PoolEvents() const
        {
            ASSERT(isInited_);

            glfwPollEvents();
        }
    }
}