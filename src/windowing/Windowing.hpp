#pragma once

#include <memory>
#include <vector>

struct SDL_Keysym;
struct SDL_MouseMotionEvent;

namespace Windowing {

    struct WindowSettings;
    class Window;

    class Windowing {
    public:

        class Listener {
        public:
            virtual void OnWindowResize(const Window &window) { (void) window; };

            virtual void OnKeyUp(const SDL_Keysym &keysym) { (void) keysym; }
            virtual void OnKeyDown(const SDL_Keysym &keysym) { (void) keysym; }

            virtual void OnMouseMotion(const SDL_MouseMotionEvent &mouseMotionEvent) { (void) mouseMotionEvent; }

            virtual void OnQuit() {};
        };

        Windowing();
        ~Windowing();

        //Use construct instead create to avoid problems with mvc
        static const std::shared_ptr<Window> ConstructWindow(const WindowSettings &settings);
        static void PoolEvents();
        static void Subscribe(Listener *listener);
        static void UnSubscribe(const Listener *listener);

    private:
        static std::vector<Listener*> listeners;
        static std::unique_ptr<Windowing> windowingInstance;
    };

}
