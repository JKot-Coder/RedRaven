#pragma once

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        struct WindowSettings;
        class Window;

        class IListener
        {
        public:
            virtual void OnWindowShown(const Window& window) { (void)window; };
            virtual void OnWindowHidden(const Window& window) { (void)window; };

            virtual void OnWindowFocusGained(const Window& window) { (void)window; };
            virtual void OnWindowFocusLost(const Window& window) { (void)window; };

            virtual void OnWindowResize(const Window& window) { (void)window; };
            /*
            virtual void OnKeyUp(const Window& window, const SDL_Keysym& keysym)
            {
                (void)window;
                (void)keysym;
            }
            virtual void OnKeyDown(const Window& window, const SDL_Keysym& keysym)
            {
                (void)window;
                (void)keysym;
            }
            */
            virtual void OnMouseMotion(const Window& window, const Vector2i& position, const Vector2i& relative)
            {
                (void)window;
                (void)position;
                (void)relative;
            }
            virtual void OnMouseButtonUp(const Window& window, uint32_t button)
            {
                (void)window;
                (void)button;
            }
            virtual void OnMouseButtonDown(const Window& window, uint32_t button)
            {
                (void)window;
                (void)button;
            }

            virtual void OnQuit() {};
        };

        class Windowing
        {
        public:
            Windowing();
            ~Windowing();

            static void PoolEvents();
            static void Subscribe(IListener* listener);
            static void UnSubscribe(const IListener* listener);

        private:
            static std::vector<IListener*> _listeners;
            static std::unique_ptr<Windowing> _windowingInstance;
        };
    }
}