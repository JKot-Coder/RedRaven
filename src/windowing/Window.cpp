#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include <SDL_events.h>

#include "common/Exception.hpp"

#include "windowing/WindowSettings.hpp"
#include "windowing/Window.hpp"
#include "Window.hpp"

namespace Windowing {

    Window::Window()
    {
    }

    Window::~Window() {
        if (window){
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }

    bool Window::Init(const WindowSettings &settings) {
        std::string windowerror;
		Uint32 windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL; //TODO add gapi flag based on current gapi

        ASSERT(!window)

        auto windowRect = settings.WindowRect;

		switch (windowRect.X)
		{
			case WindowRect::WINDOW_POSITION_UNDEFINED: windowRect.X = SDL_WINDOWPOS_UNDEFINED; break; 
			case WindowRect::WINDOW_POSITION_CENTERED: windowRect.X = SDL_WINDOWPOS_CENTERED; break;
		}

		switch (windowRect.Y)
		{
			case WindowRect::WINDOW_POSITION_UNDEFINED: windowRect.Y = SDL_WINDOWPOS_UNDEFINED; break;
			case WindowRect::WINDOW_POSITION_CENTERED: windowRect.Y = SDL_WINDOWPOS_CENTERED; break;
		}

        window = SDL_CreateWindow(settings.Title.c_str(), windowRect.X, windowRect.Y, windowRect.Width, windowRect.Height, windowflags);

        if (!window)
        {
            windowerror = std::string(SDL_GetError());
            return false;
        }

		auto* screen = SDL_GetWindowSurface(window);

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
		SDL_UpdateWindowSurface(window);

        SDL_SetWindowData(window, "WindowObject", this);
        return true;
    }

    int Window::GetWidth() const {
        ASSERT(window)

        int32_t w, h;
        SDL_GetWindowSize(window, &w, &h);
        return w;
    }

    int Window::GetHeight() const {
        int32_t w, h;
        SDL_GetWindowSize(window, &w, &h);
        return h;
    }

    void Window::SetMousePos(int x, int y) const {
        (void) x, y;
      //  SDL_WarpMouseInWindow(window, x, y);
    }

    void Window::ShowCursor(bool value) {
        auto result = SDL_ShowCursor(value ? SDL_ENABLE : SDL_DISABLE);
		
		switch (result) {
			case SDL_DISABLE: cursorIsHidden = true; break; 
			case SDL_ENABLE: cursorIsHidden = false; break; 
		}
    }

}