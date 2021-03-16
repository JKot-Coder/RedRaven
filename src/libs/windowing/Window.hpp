#pragma once

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

        class WindowImpl;

        class Window : public std::enable_shared_from_this<Window>, private NonCopyable
        {
        private:
            friend class Windowing::WindowSystem;

        public:
            using SharedPtr = std::shared_ptr<Window>;
            using SharedConstPtr = std::shared_ptr<const Window>;

            ~Window();

            int GetWidth() const;
            int GetHeight() const;
            void SetMousePos(int x, int y) const;
            void ShowCursor(bool value);

            std::any GetNativeHandle() const;

        private:
            Window();
            bool Init(const WindowDescription& description);

            std::unique_ptr<WindowImpl> impl_;
        };
    }
}