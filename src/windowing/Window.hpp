#pragma once

#include <common/VecMath.h>

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
                return window;
            }

            int GetWidth() const;
            int GetHeight() const;

            void SetMousePos(int x, int y) const;
            void ShowCursor(bool value);

            bool IsCursorHidden() const { return cursorIsHidden; }

            SDL_Window* GetSDLWindow() const { return window; };

        private:
            SDL_Window* window = nullptr;

            bool cursorIsHidden = false;
        };
    }
}