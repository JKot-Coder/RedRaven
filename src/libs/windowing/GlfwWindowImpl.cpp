#include "GlfwWindowImpl.hpp"

#include "windowing/WindowSystem.hpp"

#ifndef OS_WINDOWS
static_assert(false, "platform is not supported");
#endif // !OS_WINDOW

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace OpenDemo
{
    namespace Windowing
    {

        void GlfwWindowImpl::windowUpdateCallback(GLFWwindow* glfwWindow)
        {
#ifdef OS_WINDOWS
            const auto handle = glfwGetWin32Window(glfwWindow);
            const auto windowPtr = glfwGetWindowUserPointer(glfwWindow);
            ASSERT(windowPtr);

            auto windowImpl = static_cast<GlfwWindowImpl*>(windowPtr);
  
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(handle, &ps);

            FillRect(hdc, &ps.rcPaint, windowImpl->bgBrush_);
            EndPaint(handle, &ps);
#endif
        }

        GlfwWindowImpl::~GlfwWindowImpl()
        {
            if (!window_)
                return;

#ifdef OS_WINDOWS
            DeleteObject(bgBrush_);
#endif
        }

        bool GlfwWindowImpl::Init(const WindowDescription& description)
        {
            ASSERT(!window_);

            window_ = glfwCreateWindow(description.Width, description.Height, description.Title.c_str(), nullptr, nullptr);

            if (!window_)
                return false;

            glfwSetWindowRefreshCallback(window_, &windowUpdateCallback);
            glfwSetWindowUserPointer(window_, this);

#ifdef OS_WINDOWS
            bgBrush_ = CreateSolidBrush(RGB(0, 0, 0));
            UpdateWindow(glfwGetWin32Window(window_));
#endif

            return true;
        }

        void GlfwWindowImpl::ShowCursor(bool value)
        {
            ASSERT(window_);

            if (value)
            {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                const auto rawSupported = glfwRawMouseMotionSupported();

                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, rawSupported ? GLFW_TRUE : GLFW_FALSE);
            }
        }

        int GlfwWindowImpl::GetWidth() const
        {
            ASSERT(window_);

            int32_t w, h;
            glfwGetWindowSize(window_, &w, &h);
            return w;
        }

        int GlfwWindowImpl::GetHeight() const
        {
            ASSERT(window_);

            int32_t w, h;
            glfwGetWindowSize(window_, &w, &h);
            return h;
        }

        std::any GlfwWindowImpl::GetNativeHandle() const
        {
            ASSERT(window_);

            return glfwGetWin32Window(window_);
        }
    }
}