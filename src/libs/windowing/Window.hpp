#pragma once

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
#ifdef OS_WINDOWS
        // Little hack to avoid exposing windows.h include. Hope this will works well.
        using WindowHandle = void*;
#endif

        struct WindowDescription
        {
            U8String Title = "";
            uint32_t Width = 0;
            uint32_t Height = 0;
        };

        class IWindowImpl
        {
        public:
            virtual bool Init(const WindowDescription& description) = 0;

            virtual int GetWidth() const = 0;
            virtual int GetHeight() const = 0;

            virtual void SetMousePos(int x, int y) const = 0;
            virtual void ShowCursor(bool value) = 0;

            virtual WindowHandle GetNativeHandle() const = 0;
        };

        class Window : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;

            ~Window();

            bool Init(const WindowDescription& description);

            int GetWidth() const
            {
                ASSERT(impl_);
                return impl_->GetWidth();
            }

            int GetHeight() const
            {
                ASSERT(impl_);
                return impl_->GetHeight();
            }

            void SetMousePos(int x, int y) const
            {
                ASSERT(impl_);
                return impl_->SetMousePos(x, y);
            }

            void ShowCursor(bool value)
            {
                ASSERT(impl_);
                return impl_->ShowCursor(value);
            }

            WindowHandle GetNativeHandle() const
            {
                ASSERT(impl_);
                return impl_->GetNativeHandle();
            }

        private:
            Window();

            std::unique_ptr<IWindowImpl> impl_;
        };
    }
}