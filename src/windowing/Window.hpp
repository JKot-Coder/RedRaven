#pragma once

namespace Windowing{

    struct WindowSettings;

    class Window
    {
    public:
        Window();
        ~Window();

        bool Init(const WindowSettings &settings);
        bool IsWindow() const;
    private:
        SDL_Window* window;
    };

}