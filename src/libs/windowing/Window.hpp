#pragma once

#include "common/Math.hpp"

#include <any>

namespace OpenDemo
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
                U8String Title = "";
                uint32_t Width = 0;
                uint32_t Height = 0;
            };

        public:
            ~Window();

            inline void ShowCursor(bool value);
            inline int GetWidth() const;
            inline int GetHeight() const;
            inline std::any GetNativeHandle() const;
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
            virtual bool Init(Window::ICallbacks* callbacks, const Window::Description& description) = 0;

            virtual void ShowCursor(bool value) = 0;

            virtual int32_t GetWidth() const = 0;
            virtual int32_t GetHeight() const = 0;
            virtual std::any GetNativeHandle() const = 0;
        };

        inline void Window::ShowCursor(bool value)
        {
            ASSERT(impl_);
            impl_->ShowCursor(value);
        }

        inline int Window::GetWidth() const
        {
            ASSERT(impl_);
            return impl_->GetWidth();
        }

        inline int Window::GetHeight() const
        {
            ASSERT(impl_);
            return impl_->GetWidth();
        }

        inline std::any Window::GetNativeHandle() const
        {
            ASSERT(impl_);
            return impl_->GetNativeHandle();
        }

        inline IWindowImpl& Window::GetPrivateImpl()
        {
            ASSERT(impl_);
            return *impl_;
        }
    }
}