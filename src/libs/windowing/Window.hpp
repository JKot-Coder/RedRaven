#pragma once

#include "common/Math.hpp"

#include <any>

namespace RR
{
    namespace Windowing
    {
        class WindowSystem;
        class IWindowImpl;

        class Window final : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;

            enum class Attribute : uint32_t
            {
                Minimized,
                Maximized,
                Focused
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
            };

        public:
            ~Window();

            inline void ShowCursor(bool value);

            inline Vector2i GetSize() const;
            inline void SetSize(const Vector2i& size) const;

            inline Vector2i GetPosition() const;
            inline void SetPosition(const Vector2i& position) const;

            inline void SetTitle(const U8String& title) const;
            inline void SetWindowAlpha(float alpha) const;

            inline void Focus() const;
            inline int32_t GetWindowAttribute(Attribute attribute) const;

            inline std::any GetNativeHandle() const;
            inline std::any GetNativeHandleRaw() const;

            IWindowImpl& GetPrivateImpl();

        private:
            Window() = default;
            bool Init(ICallbacks* callbacks, const Description& description);

            std::unique_ptr<IWindowImpl> impl_;

        private:
            friend class Windowing::WindowSystem;
        };

        class IWindowImpl
        {
        public:
            virtual ~IWindowImpl() = default;

            virtual bool Init(Window::ICallbacks* callbacks, const Window::Description& description) = 0;

            virtual void ShowCursor(bool value) = 0;

            virtual Vector2i GetSize() const = 0;
            virtual void SetSize(const Vector2i& size) const = 0;

            virtual Vector2i GetPosition() const = 0;
            virtual void SetPosition(const Vector2i& position) const = 0;

            virtual void SetTitle(const U8String& title) const = 0;
            virtual void SetWindowAlpha(float alpha) const = 0;

            virtual void Focus() const = 0;
            virtual int32_t GetWindowAttribute(Window::Attribute attribute) const = 0;

            virtual std::any GetNativeHandle() const = 0;
            virtual std::any GetNativeHandleRaw() const = 0;
        };

        inline void Window::ShowCursor(bool value)
        {
            ASSERT(impl_);
            impl_->ShowCursor(value);
        }

        inline Vector2i Window::GetSize() const
        {
            ASSERT(impl_);
            return impl_->GetSize();
        }

        inline void Window::SetSize(const Vector2i& size) const
        {
            ASSERT(impl_);
            return impl_->SetSize(size);
        }

        inline Vector2i Window::GetPosition() const
        {
            ASSERT(impl_);
            return impl_->GetPosition();
        }

        inline void Window::SetPosition(const Vector2i& position) const
        {
            ASSERT(impl_);
            return impl_->SetPosition(position);
        }

        inline void Window::SetTitle(const U8String& title) const
        {
            ASSERT(impl_);
            return impl_->SetTitle(title);
        }

        inline void Window::SetWindowAlpha(float alpha) const
        {
            ASSERT(impl_);
            return impl_->SetWindowAlpha(alpha);
        }

        inline void Window::Focus() const
        {
            ASSERT(impl_);
            impl_->Focus();
        }

        inline int32_t Window::GetWindowAttribute(Window::Attribute attribute) const
        {
            ASSERT(impl_);
            return impl_->GetWindowAttribute(attribute);
        }

        inline std::any Window::GetNativeHandle() const
        {
            ASSERT(impl_);
            return impl_->GetNativeHandle();
        }

        inline std::any Window::GetNativeHandleRaw() const
        {
            ASSERT(impl_);
            return impl_->GetNativeHandleRaw();
        }

        inline IWindowImpl& Window::GetPrivateImpl()
        {
            ASSERT(impl_);
            return *impl_;
        }
    }
}