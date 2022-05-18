#pragma once

#include "common/Math.hpp"

#include <any>

namespace RR
{
    namespace Platform
    {
        class WindowSystem;

        class Window : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;

            enum class Attribute : uint32_t
            {
                Minimized,
                Maximized,
                Focused,
                MousePassthrough,
                TaskbarIcon
            };

            class ICallbacks
            {
            public:
                virtual void OnWindowShown() {};
                virtual void OnWindowHidden() {};

                virtual void OnWindowFocusGained() {};
                virtual void OnWindowFocusLost() {};

                // clang-format off
                virtual void OnWindowResize(uint32_t width, uint32_t height) { (void)width; (void)height; };
                // clang-format on
                /*
            virtual void OnKeyUp(const Window& window, const SDL_Keysym& keysym)
            {
                (void)window;
                (void)keysym;
            }
            virtual void OnKeyDown(const Window& window, const SDL_Keysym& keysym)
            {
                (void)window;
                (void)keysym;
            }
           
            virtual void OnMouseMotion(const Window& window, const Vector2i& position, const Vector2i& relative)
            {
                (void)window;
                (void)position;
                (void)relative;
            }
            virtual void OnMouseButtonUp(const Window& window, uint32_t button)
            {
                (void)window;
                (void)button;
            }
            virtual void OnMouseButtonDown(const Window& window, uint32_t button)
            {
                (void)window;
                (void)button;
            }
             */
                virtual void OnClose() {};
            };

            struct Description
            {
                U8String title = "";
                Vector2i size = { 0, 0 };

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
            virtual ~Window() = default;

            virtual bool Init(ICallbacks* callbacks, const Description& description) = 0;

            virtual void ShowCursor(bool value) = 0;

            virtual Vector2i GetSize() const = 0;
            virtual Vector2i GetFramebufferSize() const = 0;
            virtual void SetSize(const Vector2i& size) const = 0;

            virtual Vector2i GetPosition() const = 0;
            virtual void SetPosition(const Vector2i& position) const = 0;

            virtual void SetTitle(const U8String& title) const = 0;
            virtual void SetWindowAlpha(float alpha) const = 0;

            virtual int32_t GetWindowAttribute(Window::Attribute attribute) const = 0;
            virtual void SetWindowAttribute(Window::Attribute attribute, int32_t value) = 0;

            virtual void SetClipboardText(const U8String& text) const = 0;
            virtual U8String GetClipboardText() const = 0;

            virtual std::any GetNativeHandle() const = 0;
            virtual std::any GetNativeHandleRaw() const = 0;

            virtual void Focus() const = 0;
            virtual void Show() const = 0;
        };
    }
}