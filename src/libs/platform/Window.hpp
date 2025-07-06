#pragma once

#include "common/Event.hpp"
#include "math/VectorMath.hpp"
#include <any>

namespace RR::Platform
{
    using namespace RR::Common;

    struct Cursor;

    namespace Input
    {
        enum class MouseButton : uint32_t;
        enum class ModifierFlag : uint32_t;
        enum class KeyAction : uint32_t;
        enum class Key : uint32_t;
    }

    class Window : public std::enable_shared_from_this<Window>
    {
    public:
        NONCOPYABLE(Window);

        using SharedPtr = std::shared_ptr<Window>;
        using SharedConstPtr = std::shared_ptr<const Window>;

        enum class Attribute : uint32_t
        {
            Cursor,
            Focused,
            Hovered,
            Maximized,
            Minimized,
            MousePassthrough,
            TaskbarIcon,
        };

        enum class CursorState : int32_t
        {
            Normal,
            Hidden,
            Disabled,
        };

        struct Description
        {
            std::string title = "";
            Vector2i size = {0, 0};

            bool autoIconify = true;
            bool centerCursor = true;
            bool decorated = true;
            bool floating = false;
            bool focused = true;
            bool focusOnShow = true;
            bool resizable = true;
            bool visible = true;
            bool mousePassthrough = false;
            bool taskbarIcon = true;
        };

    public:
        Window() = default;
        virtual ~Window() = default;

        virtual Vector2i GetSize() const = 0;
        virtual Vector2i GetFramebufferSize() const = 0;
        virtual void SetSize(const Vector2i& size) const = 0;

        virtual Vector2i GetPosition() const = 0;
        virtual void SetPosition(const Vector2i& position) const = 0;

        virtual Vector2i GetMousePosition() const = 0;
        virtual void SetMousePosition(const Vector2i& position) const = 0;

        virtual void SetTitle(const std::string& title) const = 0;
        virtual void SetWindowAlpha(float alpha) const = 0;

        virtual int32_t GetWindowAttribute(Window::Attribute attribute) const = 0;
        virtual void SetWindowAttribute(Window::Attribute attribute, int32_t value) = 0;

        virtual void SetClipboardText(const std::string& text) const = 0;
        virtual const char* GetClipboardText() const = 0;

        virtual void ShowCursor(bool value) = 0;
        virtual void SetCursor(const std::shared_ptr<Cursor>& cursor) const = 0;

        virtual std::any GetNativeHandle() const = 0;
        virtual std::any GetNativeHandleRaw() const = 0;

        virtual void Focus() const = 0;
        virtual void Show() const = 0;

    public:
        Event<void(const Window&, char32_t ch)> OnChar;
        Event<void(const Window&)> OnClose;
        Event<void(const Window&, const Vector2i& scale)> OnContentScaleChange;
        Event<void(const Window&, bool focused)> OnFocus;
        Event<void(const Window&, Input::Key key, int32_t scancode, Input::KeyAction action, Input::ModifierFlag modifierflags)> OnKey;
        Event<void(const Window&, Input::MouseButton mouseButton, Input::KeyAction action, Input::ModifierFlag modifierflags)> OnMouseButton;
        Event<void(const Window&, bool entered)> OnMouseCross;
        Event<void(const Window&, const Vector2i& mousePosition)> OnMouseMove;
        Event<void(const Window&, const Vector2i& position)> OnMove;
        Event<void(const Window&, const Vector2i& size)> OnResize;
        Event<void(const Window&, const Vector2& wheel)> OnScroll;
    };
}