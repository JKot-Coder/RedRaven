#pragma once

#include "common/Math.hpp"
#include "common/NativeWindowHandle.hpp"

struct SDL_Window;

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

            bool IsCursorHidden() const { return _cursorIsHidden; }

            OpenDemo::Common::NativeWindowHandle GetNativeHandle() const;
            SDL_Window* GetSDLWindow() const { return _window; };

        private:
            SDL_Window* _window = nullptr;

            bool _cursorIsHidden = false;
        };
    }
}