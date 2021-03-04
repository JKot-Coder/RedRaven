#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

struct GLFWwindow;

namespace OpenDemo
{
    namespace Windowing
    {
        struct WindowSettings;

        class Window
        {
        public:
            Window();
            ~Window();

            bool Init(const WindowSettings& settings);

            inline bool IsWindow() const
            {
                return _window;
            }

            int GetWidth() const;
            int GetHeight() const;

            void SetMousePos(int x, int y) const;
            void ShowCursor(bool value);

          //  bool IsCursorHidden() const { return _cursorIsHidden; }

            OpenDemo::Common::NativeWindowHandle GetNativeHandle() const;
            GLFWwindow* GetGLFWWindow() const { return _window; };

        private:
            GLFWwindow* _window = nullptr;

         //   bool _cursorIsHidden = false;
        };
    }
}