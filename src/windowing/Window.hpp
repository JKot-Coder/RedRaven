#pragma once

#include <common/VecMath.h>

struct SDL_Window;

namespace Windowing{

    struct WindowSettings;

    class Window
    {
    public:
        Window();
        ~Window();

        bool Init(const WindowSettings &settings);
        bool IsWindow() const;

        Common::vec2 GetSize() const;

        SDL_Window* GetSDLWindow() const { return window; };
    private:
        SDL_Window* window;
    };

}