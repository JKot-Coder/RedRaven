#pragma once

#include <memory>
#include <vector>

struct SDL_Keysym;

namespace Windowing {

    struct WindowSettings;
    class Window;

    class Windowing {
    public:

        class Listener {
        public:
            virtual void WindowResize(const Window &window) { (void) window; };

            virtual void KeyUp(const SDL_Keysym &keysym) { (void) keysym; }
            virtual void KeyDown(const SDL_Keysym &keysym) { (void) keysym; }

            virtual void Quit() {};
        };

        Windowing();
        ~Windowing();

        static const std::shared_ptr<Window> CreateWindow(const WindowSettings &settings);
        static void PoolEvents();
        static void Subscribe(Listener *listener);
        static void UnSubscribe(const Listener *listener);

    private:
        static std::vector<Listener*> listeners;
        static std::unique_ptr<Windowing> windowingInstance;
    };

}
