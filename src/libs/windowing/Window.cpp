#include "Window.hpp"

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#else
static_assert(false, "Platform is not supported");
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
                glfwDestroyWindow(_window);
                _window = nullptr;
            }
        }

        bool Window::Init(const WindowSettings& settings)
        {
            U8String windowerror;
            // Uint32 windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL; //TODO add gapi flag based on current gapi

            ASSERT(!_window)

            _window = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), nullptr, nullptr);

            if (!_window)
                return false;

            // auto* screen = SDL_GetWindowSurface(_window);

            //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
            //  SDL_UpdateWindowSurface(_window);

            glfwSetWindowUserPointer(_window, this);
            return true;
        }

        int Window::GetWidth() const
        {
            ASSERT(_window)

            int32_t w, h;
            glfwGetWindowSize(_window, &w, &h);
            return w;
        }

        int Window::GetHeight() const
        {
            int32_t w, h;
            glfwGetWindowSize(_window, &w, &h);
            return h;
        }

        OpenDemo::Common::NativeWindowHandle Window::GetNativeHandle() const
        {
            if (_window)
            {
#ifdef OS_WINDOWS
                return glfwGetWin32Window(_window);
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
            ASSERT(_window);
            glfwSetInputMode(_window, GLFW_CURSOR, value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
        }
    }
}