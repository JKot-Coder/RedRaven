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
        class WindowImpl;

        class Window : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;

            struct Description
            {
                U8String Title = "";
                uint32_t Width = 0;
                uint32_t Height = 0;
            };

        public:
            ~Window();

            bool Init(const Description& description);

            int GetWidth() const;
            int GetHeight() const;

            void SetMousePos(int x, int y) const;
            void ShowCursor(bool value);

            WindowHandle GetNativeHandle() const;

        private:
            Window();

            std::unique_ptr<WindowImpl> impl_;
        };
    }
}