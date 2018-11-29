#include <SDL_syswm.h>

#include "common/Exception.hpp"
#include "windowing/WindowSettings.hpp"
#include "windowing/Window.hpp"
#include "Window.hpp"


namespace Windowing{

    Window::Window() : window(nullptr)
    {

    }

    Window::~Window() {
        if (window){
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }

    bool Window::Init(const WindowSettings &settings){
        std::string windowerror;
        Uint32 windowflags = 0;

        if (window)
            throw Common::Exception("Window already initialized");


        const auto& windowRect = settings.WindowRect;

        window = SDL_CreateWindow(settings.Title.c_str(), windowRect.X, windowRect.Y, windowRect.Width, windowRect.Height, windowflags);

        if (!window)
        {
            windowerror = std::string(SDL_GetError());
            return false;
        }

        return true;
    }

    Common::vec2 Window::GetSize() const {
        if(!window)
            return Common::vec2(0, 0);

        int32_t w, h;
        SDL_GetWindowSize(window, &w, &h);
        return Common::vec2(w, h);
    }

    bool Window::IsWindow() const{
        return window;
    }

}