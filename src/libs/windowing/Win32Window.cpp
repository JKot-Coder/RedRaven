#include "Window.hpp"

#include "windowing/WindowSystem.hpp"

#include <Windows.h>

namespace OpenDemo
{
    namespace Windowing
    {
        /*  
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
        }*/

        namespace
        {
            static DWORD getWindowStyle(const WindowDescription& description)
            {
                DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

                /* if (window->monitor)
                style |= WS_POPUP;
            else
            {
                style |= WS_SYSMENU | WS_MINIMIZEBOX;

                if (window->decorated)
                {
                    style |= WS_CAPTION;

                    if (window->resizable)
                        style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
                }
                else
                    style |= WS_POPUP;
            }*/

                return style;
            }

            static DWORD getWindowExStyle(const WindowDescription& description)
            {
                DWORD style = WS_EX_APPWINDOW;

                /*if (window->monitor || window->floating)
                    style |= WS_EX_TOPMOST;*/

                return style;
            }

        }

        class WindowImpl
        {
        public:
            HWND handle;
        };

        Window::Window()
        {
        }

        Window::~Window()
        {
        }

        bool Window::Init(const WindowDescription& description)
        {
            DWORD style = getWindowStyle(description);
            DWORD exStyle = getWindowExStyle(description);

            impl_ = std::make_unique<WindowImpl>();

            impl_->handle = CreateWindowExW(exStyle,
                                            Windowing::WINDOW_CLASS_NAME,
                                            StringConversions::UTF8ToWString(description.Title).c_str(),
                                            style,
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            description.Width, description.Height,
                                            NULL, NULL,
                                            GetModuleHandleW(NULL), 0);
            if (!impl_->handle)
                return false;

            return true;
        }

        int Window::GetWidth() const
        {
            return 0;
        }

        int Window::GetHeight() const
        {
            return 0;
        }

        void Window::SetMousePos(int x, int y) const
        {
        }

        void Window::ShowCursor(bool value)
        {
        }

        std::any Window::GetNativeHandle() const
        {
            ASSERT(impl_);

            return impl_->handle;
        }
    }
}