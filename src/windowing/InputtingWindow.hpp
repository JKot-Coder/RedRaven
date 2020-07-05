#pragma once

#include "windowing\Windowing.hpp"
#include "windowing\Window.hpp"

namespace OpenDemo
{
    namespace Inputting
    {
        class IKeyboardListener;
        class IMouseListener;
    }

    namespace Windowing
    {

        class InputtingWindow final : public Window, IListener
        {
        public:
            InputtingWindow();
            ~InputtingWindow();

            bool Init(const WindowSettings& settings, bool trapMouse_);

            void SubscribeOnKeyboardEvents(Inputting::IKeyboardListener* listener);
            void UnSubscribeOnKeyboardEvents(const Inputting::IKeyboardListener* listener);

            void SubscribeOnMouseEvents(Inputting::IMouseListener* listener);
            void UnSubscribeOnMouseEvents(const Inputting::IMouseListener* listener);

        private:
            bool _trapMouse = false;
            bool _mouseHolded = false;
            std::vector<Inputting::IKeyboardListener*> _keyboardListeners;
            std::vector<Inputting::IMouseListener*> _mouseListeners;

            virtual void OnWindowFocusLost(const Window& window) override;

            virtual void OnKeyUp(const Window& window, const SDL_Keysym& keysym) override;
            virtual void OnKeyDown(const Window& window, const SDL_Keysym& keysym) override;

            virtual void OnMouseMotion(const Window& window, const Vector2i& position, const Vector2i& relative) override;
            virtual void OnMouseButtonUp(const Window& window, uint32_t button) override;
            virtual void OnMouseButtonDown(const Window& window, uint32_t button) override;

            void HoldMouse();
            void ReleaseMouse();
        };
    }
}