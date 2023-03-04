#include "GlfwWindow.hpp"

#include "platform/Input.hpp"
#include "platform/Toolkit.hpp"
#include "platform/glfw/GlfwToolkit.hpp"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef OS_APPLE
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <optional>

namespace RR::Platform
{

    struct GlfwWindowImpl::PlatformData
    {
#ifdef OS_WINDOWS
        HBRUSH bgBrush;
#endif
    };

    namespace
    {
        std::optional<Input::MouseButton> convertMouseButton(int button)
        {
            switch (button)
            {
                case GLFW_MOUSE_BUTTON_LEFT: return Input::MouseButton::Left;
                case GLFW_MOUSE_BUTTON_RIGHT: return Input::MouseButton::Right;
                case GLFW_MOUSE_BUTTON_MIDDLE: return Input::MouseButton::Middle;
                default: return std::nullopt;
            }
        };

        Input::ModifierFlag convertMods(int mods)
        {
            Input::ModifierFlag result = Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_SHIFT) != 0 ? Input::ModifierFlag::Shift : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_CONTROL) != 0 ? Input::ModifierFlag::Control : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_ALT) != 0 ? Input::ModifierFlag::Alt : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_SUPER) != 0 ? Input::ModifierFlag::Super : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_CAPS_LOCK) != 0 ? Input::ModifierFlag::CapsLock : Input::ModifierFlag::None;
            result |= (mods & GLFW_MOD_NUM_LOCK) != 0 ? Input::ModifierFlag::NumLock : Input::ModifierFlag::None;

            return result;
        };

        Input::KeyAction convertKeyAction(int action)
        {
            switch (action)
            {
                case GLFW_PRESS: return Input::KeyAction::Press;
                case GLFW_RELEASE: return Input::KeyAction::Release;
                case GLFW_REPEAT: return Input::KeyAction::Repeat;
                default: ASSERT_MSG(false, "Unknown action"); return Input::KeyAction(-1);
            }
        }

        Input::Key convertKey(int key)
        {
            switch (key)
            {
                case GLFW_KEY_SPACE: return Input::Key::Space;
                case GLFW_KEY_APOSTROPHE: return Input::Key::Apostrophe;
                case GLFW_KEY_COMMA: return Input::Key::Comma;
                case GLFW_KEY_MINUS: return Input::Key::Minus;
                case GLFW_KEY_PERIOD: return Input::Key::Period;
                case GLFW_KEY_SLASH: return Input::Key::Slash;
                case GLFW_KEY_SEMICOLON: return Input::Key::Semicolon;
                case GLFW_KEY_EQUAL: return Input::Key::Equal;
                case GLFW_KEY_0: return Input::Key::Key0;
                case GLFW_KEY_1: return Input::Key::Key1;
                case GLFW_KEY_2: return Input::Key::Key2;
                case GLFW_KEY_3: return Input::Key::Key3;
                case GLFW_KEY_4: return Input::Key::Key4;
                case GLFW_KEY_5: return Input::Key::Key5;
                case GLFW_KEY_6: return Input::Key::Key6;
                case GLFW_KEY_7: return Input::Key::Key7;
                case GLFW_KEY_8: return Input::Key::Key8;
                case GLFW_KEY_9: return Input::Key::Key9;
                case GLFW_KEY_A: return Input::Key::A;
                case GLFW_KEY_B: return Input::Key::B;
                case GLFW_KEY_C: return Input::Key::C;
                case GLFW_KEY_D: return Input::Key::D;
                case GLFW_KEY_E: return Input::Key::E;
                case GLFW_KEY_F: return Input::Key::F;
                case GLFW_KEY_G: return Input::Key::G;
                case GLFW_KEY_H: return Input::Key::H;
                case GLFW_KEY_I: return Input::Key::I;
                case GLFW_KEY_J: return Input::Key::J;
                case GLFW_KEY_K: return Input::Key::K;
                case GLFW_KEY_L: return Input::Key::L;
                case GLFW_KEY_M: return Input::Key::M;
                case GLFW_KEY_N: return Input::Key::N;
                case GLFW_KEY_O: return Input::Key::O;
                case GLFW_KEY_P: return Input::Key::P;
                case GLFW_KEY_Q: return Input::Key::Q;
                case GLFW_KEY_R: return Input::Key::R;
                case GLFW_KEY_S: return Input::Key::S;
                case GLFW_KEY_T: return Input::Key::T;
                case GLFW_KEY_U: return Input::Key::U;
                case GLFW_KEY_V: return Input::Key::V;
                case GLFW_KEY_W: return Input::Key::W;
                case GLFW_KEY_X: return Input::Key::X;
                case GLFW_KEY_Y: return Input::Key::Y;
                case GLFW_KEY_Z: return Input::Key::Z;
                case GLFW_KEY_LEFT_BRACKET: return Input::Key::LeftBracket;
                case GLFW_KEY_BACKSLASH: return Input::Key::Backslash;
                case GLFW_KEY_RIGHT_BRACKET: return Input::Key::RightBracket;
                case GLFW_KEY_GRAVE_ACCENT: return Input::Key::GraveAccent;
                case GLFW_KEY_ESCAPE: return Input::Key::Escape;
                case GLFW_KEY_TAB: return Input::Key::Tab;
                case GLFW_KEY_ENTER: return Input::Key::Enter;
                case GLFW_KEY_BACKSPACE: return Input::Key::Backspace;
                case GLFW_KEY_INSERT: return Input::Key::Insert;
                case GLFW_KEY_DELETE: return Input::Key::Delete;
                case GLFW_KEY_RIGHT: return Input::Key::Right;
                case GLFW_KEY_LEFT: return Input::Key::Left;
                case GLFW_KEY_DOWN: return Input::Key::Down;
                case GLFW_KEY_UP: return Input::Key::Up;
                case GLFW_KEY_PAGE_UP: return Input::Key::PageUp;
                case GLFW_KEY_PAGE_DOWN: return Input::Key::PageDown;
                case GLFW_KEY_HOME: return Input::Key::Home;
                case GLFW_KEY_END: return Input::Key::End;
                case GLFW_KEY_CAPS_LOCK: return Input::Key::CapsLock;
                case GLFW_KEY_SCROLL_LOCK: return Input::Key::ScrollLock;
                case GLFW_KEY_NUM_LOCK: return Input::Key::NumLock;
                case GLFW_KEY_PRINT_SCREEN: return Input::Key::PrintScreen;
                case GLFW_KEY_PAUSE: return Input::Key::Pause;
                case GLFW_KEY_F1: return Input::Key::F1;
                case GLFW_KEY_F2: return Input::Key::F2;
                case GLFW_KEY_F3: return Input::Key::F3;
                case GLFW_KEY_F4: return Input::Key::F4;
                case GLFW_KEY_F5: return Input::Key::F5;
                case GLFW_KEY_F6: return Input::Key::F6;
                case GLFW_KEY_F7: return Input::Key::F7;
                case GLFW_KEY_F8: return Input::Key::F8;
                case GLFW_KEY_F9: return Input::Key::F9;
                case GLFW_KEY_F10: return Input::Key::F10;
                case GLFW_KEY_F11: return Input::Key::F11;
                case GLFW_KEY_F12: return Input::Key::F12;
                case GLFW_KEY_KP_0: return Input::Key::Keypad0;
                case GLFW_KEY_KP_1: return Input::Key::Keypad1;
                case GLFW_KEY_KP_2: return Input::Key::Keypad2;
                case GLFW_KEY_KP_3: return Input::Key::Keypad3;
                case GLFW_KEY_KP_4: return Input::Key::Keypad4;
                case GLFW_KEY_KP_5: return Input::Key::Keypad5;
                case GLFW_KEY_KP_6: return Input::Key::Keypad6;
                case GLFW_KEY_KP_7: return Input::Key::Keypad7;
                case GLFW_KEY_KP_8: return Input::Key::Keypad8;
                case GLFW_KEY_KP_9: return Input::Key::Key9;
                case GLFW_KEY_KP_DECIMAL: return Input::Key::KeypadDecimal;
                case GLFW_KEY_KP_DIVIDE: return Input::Key::KeypadDivide;
                case GLFW_KEY_KP_MULTIPLY: return Input::Key::KeypadMultiply;
                case GLFW_KEY_KP_SUBTRACT: return Input::Key::KeypadSubtract;
                case GLFW_KEY_KP_ADD: return Input::Key::KeypadAdd;
                case GLFW_KEY_KP_ENTER: return Input::Key::KeypadEnter;
                case GLFW_KEY_KP_EQUAL: return Input::Key::KeypadEqual;
                case GLFW_KEY_LEFT_SHIFT: return Input::Key::LeftShift;
                case GLFW_KEY_LEFT_CONTROL: return Input::Key::LeftControl;
                case GLFW_KEY_LEFT_ALT: return Input::Key::LeftAlt;
                case GLFW_KEY_LEFT_SUPER: return Input::Key::LeftSuper;
                case GLFW_KEY_RIGHT_SHIFT: return Input::Key::RightShift;
                case GLFW_KEY_RIGHT_CONTROL: return Input::Key::RightControl;
                case GLFW_KEY_RIGHT_ALT: return Input::Key::RightAlt;
                case GLFW_KEY_RIGHT_SUPER: return Input::Key::RightSuper;
                case GLFW_KEY_MENU: return Input::Key::Menu;
                default: return Input::Key::Unknown;
            }
        }
    }

    GlfwWindowImpl::GlfwWindowImpl()
        : platformData_(new PlatformData())
    {
    }

    GlfwWindowImpl::~GlfwWindowImpl()
    {
        if (!window_)
            return;

        glfwDestroyWindow(window_);
#ifdef OS_WINDOWS
        DeleteObject(platformData_->bgBrush);
#endif
    }

    void GlfwWindowImpl::charCallback(GLFWwindow* glfwWindow, unsigned int c)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnChar.Dispatch(*windowImpl, c);
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

    void GlfwWindowImpl::keyCallback(GLFWwindow* glfwWindow, int keycode, int scancode, int action, int mods)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        // Workaround: X11 does not include current pressed/released modifier key in 'mods' flags. https://github.com/glfw/glfw/issues/1630
        // if (int keycode_to_mod = ImGui_ImplGlfw_KeyToModifier(keycode))
        //   mods = (action == Input::KeyAction::Press) ? (mods | keycode_to_mod) : (mods & ~keycode_to_mod);

        const auto key = convertKey(keycode);
        const auto modifierFlags = convertMods(mods);
        const auto keyAction = convertKeyAction(action);

        windowImpl->OnKey.Dispatch(*windowImpl, key, scancode, keyAction, modifierFlags);
    }

    void GlfwWindowImpl::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
    {
        if (action != GLFW_PRESS && action != GLFW_RELEASE)
            return;

        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        const auto mouseButton = convertMouseButton(button);
        const auto keyAction = convertKeyAction(action);

        if (!mouseButton)
            return;

        const auto modifierFlags = convertMods(mods);
        windowImpl->OnMouseButton.Dispatch(*windowImpl, mouseButton.value(), keyAction, modifierFlags);
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

    void GlfwWindowImpl::windowContentScale(GLFWwindow* glfwWindow, float xscale, float yscale)
    {
        const auto windowImpl = static_cast<GlfwWindowImpl*>(glfwGetWindowUserPointer(glfwWindow));
        ASSERT(windowImpl);

        windowImpl->OnContentScaleChange.Dispatch(*windowImpl, Vector2(xscale, yscale));
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

        FillRect(hdc, &ps.rcPaint, windowImpl->platformData_->bgBrush);
        EndPaint(handle, &ps);
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

        glfwSetKeyCallback(window_, &keyCallback);
        glfwSetCharCallback(window_, &charCallback);
        glfwSetCursorEnterCallback(window_, &cursorEnterCallback);
        glfwSetCursorPosCallback(window_, &cursorPosCallback);
        glfwSetMouseButtonCallback(window_, &mouseButtonCallback);
        glfwSetScrollCallback(window_, &scrollCallback);
        glfwSetWindowCloseCallback(window_, &windowCloseCallback);
        glfwSetWindowContentScaleCallback(window_, &windowContentScale);
        glfwSetWindowFocusCallback(window_, &windowFocusCallback);
        glfwSetWindowPosCallback(window_, &windowPosCallback);
        glfwSetWindowRefreshCallback(window_, &windowUpdateCallback);
        glfwSetWindowSizeCallback(window_, &windowResizeCallback);

        glfwSetWindowUserPointer(window_, this);

#ifdef OS_WINDOWS
        platformData_->bgBrush = CreateSolidBrush(RGB(0, 0, 0));
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

    void GlfwWindowImpl::SetCursor(const std::shared_ptr<Cursor>& cursor) const
    {
        ASSERT(window_);
        ASSERT(cursor);

        glfwSetCursor(window_, std::static_pointer_cast<GlfwCursor>(cursor)->cursor);
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
                    case GLFW_CURSOR_NORMAL: return static_cast<int32_t>(CursorState::Normal);
                    case GLFW_CURSOR_HIDDEN: return static_cast<int32_t>(CursorState::Hidden);
                    case GLFW_CURSOR_DISABLED: return static_cast<int32_t>(CursorState::Disabled);
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
                switch (static_cast<CursorState>(value))
                {
                    case CursorState::Normal: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    case CursorState::Hidden: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                    case CursorState::Disabled: return glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
        // TODO temporary disable cause it provides stall in multithreading
        // SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle);

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