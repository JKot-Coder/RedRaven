#pragma once

#include <common/VecMath.h>

namespace Windowing{

    struct WindowSettings;

    class Window
    {
    public:
        Window();
        ~Window();

        bool Init(const WindowSettings &settings);
        bool IsWindow() const;


        Common::vec2 GetSize() const {
            if(!window)
                return Common::vec2(0, 0);

            int32_t w, h;
            SDL_GetWindowSize(window, &w, &h);
            return Common::vec2(w, h);
        }

        SDL_Window* GetSDLWindow() const { return window; };
    private:
        SDL_Window* window;
    };

}