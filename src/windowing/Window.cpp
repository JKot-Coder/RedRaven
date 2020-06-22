#include "Window.hpp"

#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_syswm.h>

#include "common/Exception.hpp"

#include "windowing/Window.hpp"
#include "windowing/WindowSettings.hpp"

namespace OpenDemo
{
    namespace Windowing
    {

        Window::Window()
        {
        }

        Window::~Window()
        {
            if (_window)
            {
                SDL_DestroyWindow(_window);
                _window = nullptr;
            }
        }

        bool Window::Init(const WindowSettings& settings)
        {
            U8String windowerror;
            Uint32 windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL; //TODO add gapi flag based on current gapi

            ASSERT(!_window)

            auto windowRect = settings.WindowRect;

            switch (windowRect.X)
            {
            case WindowRect::WINDOW_POSITION_UNDEFINED:
                windowRect.X = SDL_WINDOWPOS_UNDEFINED;
                break;
            case WindowRect::WINDOW_POSITION_CENTERED:
                windowRect.X = SDL_WINDOWPOS_CENTERED;
                break;
            }

            switch (windowRect.Y)
            {
            case WindowRect::WINDOW_POSITION_UNDEFINED:
                windowRect.Y = SDL_WINDOWPOS_UNDEFINED;
                break;
            case WindowRect::WINDOW_POSITION_CENTERED:
                windowRect.Y = SDL_WINDOWPOS_CENTERED;
                break;
            }

            _window = SDL_CreateWindow(settings.Title.c_str(), windowRect.X, windowRect.Y, windowRect.Width, windowRect.Height, windowflags);

            if (!_window)
            {
                windowerror = U8String(SDL_GetError());
                return false;
            }

            auto* screen = SDL_GetWindowSurface(_window);

            SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
            SDL_UpdateWindowSurface(_window);

            SDL_SetWindowData(_window, "WindowObject", this);
            return true;
        }

        int Window::GetWidth() const
        {
            ASSERT(_window)

            int32_t w, h;
            SDL_GetWindowSize(_window, &w, &h);
            return w;
        }

        int Window::GetHeight() const
        {
            int32_t w, h;
            SDL_GetWindowSize(_window, &w, &h);
            return h;
        }

        OpenDemo::Common::NativeWindowHandle Window::GetNativeHandle() const
        {
            if (_window)
            {
                SDL_SysWMinfo wmInfo;
                SDL_VERSION(&wmInfo.version);
                SDL_GetWindowWMInfo(_window, &wmInfo);
#ifdef OS_WINDOWS
                return wmInfo.info.win.window;
#endif
            }

            return 0;
        }

        void Window::SetMousePos(int x, int y) const
        {
            (void)x, y;
            //  SDL_WarpMouseInWindow(window, x, y);
        }

        void Window::ShowCursor(bool value)
        {
            auto result = SDL_ShowCursor(value ? SDL_ENABLE : SDL_DISABLE);

            switch (result)
            {
            case SDL_DISABLE:
                _cursorIsHidden = true;
                break;
            case SDL_ENABLE:
                _cursorIsHidden = false;
                break;
            }
        }

    }
}