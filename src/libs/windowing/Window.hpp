#pragma once

#include "common/EventProvider.hpp"
#include "common/Math.hpp"

#include <any>

namespace OpenDemo
{
    namespace Windowing
    {
        class WindowSystem;

        struct WindowDescription
        {
            U8String Title = "";
            uint32_t Width = 0;
            uint32_t Height = 0;
        };

        enum class WindowEvent : uint32_t
        {
            CLOSE
        };

        class IWindowImpl : public EventProvider<WindowEvent>
        {
        public:
            virtual bool Init(const WindowDescription& description) = 0;

            virtual void ShowCursor(bool value) = 0;

            virtual int32_t GetWidth() const = 0;
            virtual int32_t GetHeight() const = 0;
            virtual std::any GetNativeHandle() const = 0;
        };

        class Window final : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        private:
            friend class Windowing::WindowSystem;

        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;
            using CallBackFunction = EventProvider<WindowEvent>::CallBackFunction;

            ~Window();

            inline void Subscribe(WindowEvent eventType, CallBackFunction callback)
            {
                ASSERT(impl_);
                impl_->Subscribe(eventType, callback);
            }

            inline void Unsubscribe(WindowEvent eventType, CallBackFunction callback)
            {
                ASSERT(impl_);
                impl_->Unsubscribe(eventType, callback);
            }

            inline void ShowCursor(bool value)
            {
                ASSERT(impl_);
                impl_->ShowCursor(value);
            }

            inline int GetWidth() const
            {
                ASSERT(impl_);
                return impl_->GetWidth();
            }

            inline int GetHeight() const
            {
                ASSERT(impl_);
                return impl_->GetWidth();
            }

            inline std::any GetNativeHandle() const
            {
                ASSERT(impl_);
                return impl_->GetNativeHandle();
            }

            IWindowImpl& GetPrivateImpl()
            {
                ASSERT(impl_);
                return *impl_;
            }

        private:
            Window() = default;
            bool Init(const WindowDescription& description);

            std::unique_ptr<IWindowImpl> impl_;
        };
    }
}