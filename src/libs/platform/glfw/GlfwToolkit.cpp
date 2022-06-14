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

        GlfwCursor ::~GlfwCursor() { glfwDestroyCursor(cursor); }

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

        std::shared_ptr<Cursor> GlfwToolkit::CreateCursor(Cursor::Type type) const
        {
            ASSERT(isInited_);

            auto cursor = cursors_[size_t(type)].lock();

            if (cursor != nullptr)
                return cursor;

            // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
            // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
            // Missing cursors will return nullptr.
            GLFWerrorfun prevErrorCallback = glfwSetErrorCallback(nullptr);

            GLFWcursor* glfwCursor = nullptr;
            switch (type)
            {
                case Cursor::Type::Arrow: glfwCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); break;
                case Cursor::Type::IBeam: glfwCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR); break;
                case Cursor::Type::Crosshair: glfwCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR); break;
                case Cursor::Type::Hand: glfwCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR); break;
                case Cursor::Type::HResize: glfwCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR); break;
                case Cursor::Type::VResize: glfwCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR); break;
                default: ASSERT_MSG(false, "Unknown cursor");
            }

            glfwSetErrorCallback(prevErrorCallback);

            if (glfwCursor)
            {
                cursor = std::make_shared<GlfwCursor>(glfwCursor);
                cursors_[size_t(type)] = cursor; // Caching
            }

            return cursor;
        }

        void GlfwToolkit::PoolEvents() const
        {
            ASSERT(isInited_);

            glfwPollEvents();
        }
    }
}