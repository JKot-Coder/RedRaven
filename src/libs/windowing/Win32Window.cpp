#include "Window.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        Window::Window()
        {
        }

        Window::~Window()
        {
        }

        bool Window::Init(const Description& description)
        {
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
            RECT area;
            GetClientRect(window->win32.handle, &area);

            if (width)
                *width = area.right;
            if (height)
                *height = area.bottom;

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

        WindowHandle Window::GetNativeHandle() const
        {
            return 0;
        }

        void Window::SetMousePos(int x, int y) const
        {

        }

        void Window::ShowCursor(bool value)
        {
            ASSERT(_window);
            glfwSetInputMode(_window, GLFW_CURSOR, value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
        }
    }
}