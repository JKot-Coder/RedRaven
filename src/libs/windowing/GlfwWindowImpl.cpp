#include "GlfwWindowImpl.hpp"

#include "windowing/WindowSystem.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#ifndef OS_WINDOWS
static_assert(false, "platform is not supported");
#endif // !OS_WINDOW

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace RR
{
    namespace Windowing
    {
        void GlfwWindowImpl::windowUpdateCallback(GLFWwindow* glfwWindow)
        {
#ifdef OS_WINDOWS
            const auto handle = glfwGetWin32Window(glfwWindow);

            const auto windowPtr = glfwGetWindowUserPointer(glfwWindow);
            auto windowImpl = static_cast<GlfwWindowImpl*>(windowPtr);
            ASSERT(windowImpl);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(handle, &ps);

            FillRect(hdc, &ps.rcPaint, windowImpl->bgBrush_);
            EndPaint(handle, &ps);
#endif
        }

        void GlfwWindowImpl::windowResizeCallback(GLFWwindow* glfwWindow, int width, int height)
        {
            ASSERT(width >= 0);
            ASSERT(height >= 0);

            const auto windowPtr = glfwGetWindowUserPointer(glfwWindow);
            auto windowImpl = static_cast<GlfwWindowImpl*>(windowPtr);
            ASSERT(windowImpl);

            if (windowImpl->callbacks_)
                windowImpl->callbacks_->OnWindowResize(static_cast<uint32_t>(width),
                                                       static_cast<uint32_t>(height));
        }

        void GlfwWindowImpl::windowCloseCallback(GLFWwindow* glfwWindow)
        {
            const auto windowPtr = glfwGetWindowUserPointer(glfwWindow);
            auto windowImpl = static_cast<GlfwWindowImpl*>(windowPtr);
            ASSERT(windowImpl);

            if (windowImpl->callbacks_)
                windowImpl->callbacks_->OnClose();
        }

        GlfwWindowImpl::~GlfwWindowImpl()
        {
            if (!window_)
                return;

            glfwDestroyWindow(window_);
#ifdef OS_WINDOWS
            DeleteObject(bgBrush_);
#endif
        }

        bool GlfwWindowImpl::Init(Window::ICallbacks* callbacks, const Window::Description& description)
        {
            ASSERT(!window_);
            //   ASSERT(callbacks);

            callbacks_ = callbacks;

            glfwWindowHint(GLFW_AUTO_ICONIFY, description.autoIconify);
            glfwWindowHint(GLFW_CENTER_CURSOR, description.centerCursor);
            glfwWindowHint(GLFW_DECORATED, description.decorated);
            glfwWindowHint(GLFW_FLOATING, description.floating);
            glfwWindowHint(GLFW_FOCUS_ON_SHOW, description.focusOnShow);
            glfwWindowHint(GLFW_FOCUSED, description.focused);
            glfwWindowHint(GLFW_RESIZABLE, description.resizable);
            glfwWindowHint(GLFW_VISIBLE, description.visible);

            window_ = glfwCreateWindow(description.size.x, description.size.y, description.title.c_str(), nullptr, nullptr);

            if (description.mousePassthrough)
                setWindowMousePassthrough(true);

            if (!description.taskbarIcon)
                setTaskbarIcon(false);

            glfwDefaultWindowHints();

            if (!window_)
                return false;

            glfwSetWindowRefreshCallback(window_, &windowUpdateCallback);
            glfwSetWindowSizeCallback(window_, &windowResizeCallback);
            glfwSetWindowCloseCallback(window_, &windowCloseCallback);
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

        Vector2i GlfwWindowImpl::GetSize() const
        {
            ASSERT(window_);

            int32_t w, h;
            glfwGetWindowSize(window_, &w, &h);
            return Vector2i(w, h);
        }

        void GlfwWindowImpl::SetSize(const Vector2i& size) const
        {
            ASSERT(window_);

            glfwSetWindowSize(window_, size.x, size.y);
        }

        Vector2i GlfwWindowImpl::GetPosition() const
        {
            ASSERT(window_);

            int32_t x, y;
            glfwGetWindowPos(window_, &x, &y);
            return Vector2i(x, y);
        }

        void GlfwWindowImpl::SetPosition(const Vector2i& position) const
        {
            ASSERT(window_);

            glfwSetWindowPos(window_, position.x, position.y);
        }

        void GlfwWindowImpl::SetTitle(const U8String& title) const
        {
            ASSERT(window_);

            glfwSetWindowTitle(window_, title.c_str());
        }

        void GlfwWindowImpl::SetWindowAlpha(float alpha) const
        {
            ASSERT(window_);

            glfwSetWindowOpacity(window_, alpha);
        }

        void GlfwWindowImpl::Focus() const
        {
            ASSERT(window_);

            glfwFocusWindow(window_);
        }

        int32_t GlfwWindowImpl::GetWindowAttribute(Window::Attribute attribute) const
        {
            ASSERT(window_);

            switch (attribute)
            {
                case RR::Windowing::Window::Attribute::Minimized:
                    return glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0;
                case RR::Windowing::Window::Attribute::Maximized:
                    return glfwGetWindowAttrib(window_, GLFW_MAXIMIZED) != 0;
                case RR::Windowing::Window::Attribute::Focused:
                    return glfwGetWindowAttrib(window_, GLFW_FOCUSED) != 0;
                case RR::Windowing::Window::Attribute::MousePassthrough:
                    return mousePassthrough_;
                case RR::Windowing::Window::Attribute::TaskbarIcon:
                    return taskbarIcon_;
                default:
                    ASSERT_MSG(false, "Unknown window attribute");
            }

            return 0;
        }

        void GlfwWindowImpl::SetWindowAttribute(Window::Attribute attribute, int32_t value)
        {
            ASSERT(window_);

            switch (attribute)
            {
                case RR::Windowing::Window::Attribute::Minimized:
                    return glfwSetWindowAttrib(window_, GLFW_ICONIFIED, value != 0);
                case RR::Windowing::Window::Attribute::Maximized:
                    return glfwSetWindowAttrib(window_, GLFW_MAXIMIZED, value != 0);
                case RR::Windowing::Window::Attribute::Focused:
                    return glfwSetWindowAttrib(window_, GLFW_FOCUSED, value != 0);
                case RR::Windowing::Window::Attribute::MousePassthrough:
                    return setWindowMousePassthrough(value != 0);
                case RR::Windowing::Window::Attribute::TaskbarIcon:
                    return setTaskbarIcon(value != 0);
                default:
                    ASSERT_MSG(false, "Unknown window attribute");
            }
        }

        std::any GlfwWindowImpl::GetNativeHandle() const
        {
            ASSERT(window_);

            return window_;
        }

        std::any GlfwWindowImpl::GetNativeHandleRaw() const
        {
            ASSERT(window_);

            return glfwGetWin32Window(window_);
        }

#ifdef OS_WINDOWS
        // We have submitted https://github.com/glfw/glfw/pull/1568 to allow GLFW to support "transparent inputs".
        // In the meanwhile we implement custom per-platform workarounds here (FIXME-VIEWPORT: Implement same work-around for Linux/OSX!)
        void GlfwWindowImpl::setWindowMousePassthrough(bool enabled)
        {
            COLORREF key = 0;
            BYTE alpha = 0;
            DWORD flags = 0;
            HWND hwnd = glfwGetWin32Window(window_);
            DWORD exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);

            if (exStyle & WS_EX_LAYERED)
                GetLayeredWindowAttributes(hwnd, &key, &alpha, &flags);

            if (enabled)
                exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
            else
            {
                exStyle &= ~WS_EX_TRANSPARENT;
                // NOTE: Window opacity and framebuffer transparency also need to
                //       control the layered style so avoid stepping on their feet
                if (exStyle & WS_EX_LAYERED)
                {
                    if (!(flags & (LWA_ALPHA | LWA_COLORKEY)))
                        exStyle &= ~WS_EX_LAYERED;
                }
            }

            SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle);

            if (enabled)
                SetLayeredWindowAttributes(hwnd, key, alpha, flags);

            mousePassthrough_ = enabled;
        }
#else
        void GlfwWindowImpl::setWindowMousePassthrough(bool enabled)
        {
            mousePassthrough_ = enabled;

            ASSERT_MSG(false, "Not implemented");
        }
#endif

#ifdef OS_WINDOWS
        void GlfwWindowImpl::setTaskbarIcon(bool enabled)
        {
            // GLFW hack: Hide icon from task bar
            HWND hwnd = glfwGetWin32Window(window_);

            LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);

            if (enabled)
            {
                ex_style &= ~WS_EX_TOOLWINDOW;
                ex_style |= WS_EX_APPWINDOW;
            }
            else
            {
                ex_style &= ~WS_EX_APPWINDOW;
                ex_style |= WS_EX_TOOLWINDOW;
            }

            ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
            taskbarIcon_ = enabled;
        }
#else
        void GlfwWindowImpl::setTaskbarIcon(bool enabled)
        {
            taskbarIcon_ = enabled;

            ASSERT_MSG(false, "Not implemented");
        }
#endif

    }
}