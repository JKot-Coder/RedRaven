#include "GlfwWindowImpl.hpp"

#include "platform/Input.hpp"
#include "platform/Toolkit.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef OS_APPLE
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

namespace RR::Platform
{
    namespace
    {
        Input::MouseButton convertMouseButton(int button)
        {
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT: return Input::MouseButton::Left;
                case GLFW_MOUSE_BUTTON_RIGHT: return Input::MouseButton::Right;
                case GLFW_MOUSE_BUTTON_MIDDLE: return Input::MouseButton::Middle;
                default: return Input::MouseButton::Unknown;
            }
        };

        Input::ModifierFlag convertMods(int mods)
        {
            Input::ModifierFlag result = Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_SHIFT != 0) ? Input::ModifierFlag::Shift : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_CONTROL != 0) ? Input::ModifierFlag::Control : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_ALT != 0) ? Input::ModifierFlag::Alt : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_SUPER != 0) ? Input::ModifierFlag::Super : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_CAPS_LOCK != 0) ? Input::ModifierFlag::CapsLock : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_NUM_LOCK != 0) ? Input::ModifierFlag::NumLock : Input::ModifierFlag::None;

            return result;
        };
    }

    void GlfwWindowImpl::cursorEnterCallback(GLFWwindow* glfwWindow, int entered)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnMouseCross.Dispatch(*windowImpl, entered != 0);
    }

    void GlfwWindowImpl::cursorPosCallback(GLFWwindow* glfwWindow, double x, double y)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnMouseMove.Dispatch(*windowImpl, Vector2(static_cast<int32_t>(x),
                                                              static_cast<int32_t>(y)));
    }

    void GlfwWindowImpl::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
    {
        if (action != GLFW_PRESS && action != GLFW_RELEASE)
            return;

        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        const auto mouseButton = convertMouseButton(button);

        if (mouseButton == Input::MouseButton::Unknown)
            return;

        const auto modifierFlags = convertMods(mods);
        windowImpl->OnMouseButton.Dispatch(*windowImpl, mouseButton, modifierFlags, action == GLFW_PRESS);
    }

    void GlfwWindowImpl::scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnScroll.Dispatch(*windowImpl, Vector2(xoffset, yoffset));
    }

    void GlfwWindowImpl::windowCloseCallback(GLFWwindow* glfwWindow)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnClose.Dispatch(*windowImpl);
    }

    void GlfwWindowImpl::windowFocusCallback(GLFWwindow* glfwWindow, int focused)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnFocus.Dispatch(*windowImpl, focused != 0);
    }

    void GlfwWindowImpl::windowPosCallback(GLFWwindow* glfwWindow, int x, int y)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnMove.Dispatch(*windowImpl, Vector2i(x, y));
    }

    void GlfwWindowImpl::windowResizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        ASSERT(width >= 0);
        ASSERT(height >= 0);

        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnResize.Dispatch(*windowImpl, Vector2i(width, height));
    }

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

    GlfwWindowImpl::~GlfwWindowImpl()
    {
        if (!window_)
            return;

        glfwDestroyWindow(window_);
#ifdef OS_WINDOWS
        DeleteObject(bgBrush_);
#endif
    }

    bool GlfwWindowImpl::init(const Description& description)
    {
        ASSERT(!window_);

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

        glfwSetCursorEnterCallback(window_, &cursorEnterCallback);
        glfwSetCursorPosCallback(window_, &cursorPosCallback);
        glfwSetMouseButtonCallback(window_, &mouseButtonCallback);
        glfwSetScrollCallback(window_, &scrollCallback);
        glfwSetWindowCloseCallback(window_, &windowCloseCallback);
        glfwSetWindowFocusCallback(window_, &windowFocusCallback);
        glfwSetWindowPosCallback(window_, &windowPosCallback);
        glfwSetWindowRefreshCallback(window_, &windowUpdateCallback);
        glfwSetWindowSizeCallback(window_, &windowResizeCallback);

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

    Vector2i GlfwWindowImpl::GetFramebufferSize() const
    {
        ASSERT(window_);

        int32_t w, h;
        glfwGetFramebufferSize(window_, &w, &h);
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

    Vector2i GlfwWindowImpl::GetMousePosition() const
    {
        ASSERT(window_);

        Vector<2, double> mousePos;
        glfwGetCursorPos(window_, &mousePos.x, &mousePos.y);

        return mousePos.Cast<int32_t>();
    }

    void GlfwWindowImpl::SetMousePosition(const Vector2i& position) const
    {
        ASSERT(window_);

        glfwSetCursorPos(window_, position.x, position.y);
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

    int32_t GlfwWindowImpl::GetWindowAttribute(Attribute attribute) const
    {
        ASSERT(window_);

        switch (attribute)
        {
            case Attribute::Cursor:
            {
                switch (glfwGetInputMode(window_, GLFW_CURSOR))
                {
                    case GLFW_CURSOR_NORMAL: return static_cast<int32_t>(Cursor::Normal);
                    case GLFW_CURSOR_HIDDEN: return static_cast<int32_t>(Cursor::Hidden);
                    case GLFW_CURSOR_DISABLED: return static_cast<int32_t>(Cursor::Disabled);
                    default: ASSERT_MSG(false, "Unknown input mode"); return -1;
                };
                break;
            }
            case Attribute::Focused: return glfwGetWindowAttrib(window_, GLFW_FOCUSED) != 0;
            case Attribute::Hovered: return glfwGetWindowAttrib(window_, GLFW_HOVERED) != 0;
            case Attribute::Maximized: return glfwGetWindowAttrib(window_, GLFW_MAXIMIZED) != 0;
            case Attribute::Minimized: return glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0;
            case Attribute::MousePassthrough: return mousePassthrough_;
            case Attribute::TaskbarIcon: return taskbarIcon_;
            default: ASSERT_MSG(false, "Unknown window attribute"); return -1;
        }

        return -1;
    }

    void GlfwWindowImpl::SetWindowAttribute(Attribute attribute, int32_t value)
    {
        ASSERT(window_);

        switch (attribute)
        {
            case Attribute::Cursor:
            {
                switch (static_cast<Cursor>(value))
                {
                    case Cursor::Normal: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    case Cursor::Hidden: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                    case Cursor::Disabled: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    default: ASSERT_MSG(false, "Unknown cursor mode"); return;
                };
                break;
            }
            case Attribute::Minimized: return glfwSetWindowAttrib(window_, GLFW_ICONIFIED, value != 0);
            case Attribute::Maximized: return glfwSetWindowAttrib(window_, GLFW_MAXIMIZED, value != 0);
            case Attribute::Focused: return glfwSetWindowAttrib(window_, GLFW_FOCUSED, value != 0);
            case Attribute::MousePassthrough: return setWindowMousePassthrough(value != 0);
            case Attribute::TaskbarIcon: return setTaskbarIcon(value != 0);
            default: ASSERT_MSG(false, "Invalid attribute to set");
        }
    }

    U8String GlfwWindowImpl::GetClipboardText() const
    {
        ASSERT(window_);

        return glfwGetClipboardString(window_);
    }

    void GlfwWindowImpl::SetClipboardText(const U8String& text) const
    {
        ASSERT(window_);

        glfwSetClipboardString(window_, text.c_str());
    }

    std::any GlfwWindowImpl::GetNativeHandle() const
    {
        ASSERT(window_);

        return window_;
    }

    std::any GlfwWindowImpl::GetNativeHandleRaw() const
    {
        ASSERT(window_);

#if OS_WINDOWS
        return glfwGetWin32Window(window_);
#elif OS_APPLE
        return glfwGetCocoaWindow(window_);
#else
        return nullptr;
#endif
    }

    void GlfwWindowImpl::Focus() const
    {
        ASSERT(window_);

        glfwFocusWindow(window_);
    }

    void GlfwWindowImpl::Show() const
    {
        ASSERT(window_);

        glfwShowWindow(window_);
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

    std::shared_ptr<Window> GlfwWindowImpl::Create(const Window::Description& description)
    {
        const auto& window = new GlfwWindowImpl();

        if (!window->init(description))
            return nullptr;

        return std::shared_ptr<Window>(window);
    }
}