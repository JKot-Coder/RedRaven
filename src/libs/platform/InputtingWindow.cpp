#include "InputtingWindow.hpp"

#include "common/Exception.hpp"

#include "inputting/Input.hpp"

#include "windowing/Windowing.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        /*
        static const std::map<SDL_Scancode, Inputting::InputKey> SDLToInputKeyMap = {
            { SDL_SCANCODE_A, Inputting::ikA },
            { SDL_SCANCODE_B, Inputting::ikB },
            { SDL_SCANCODE_C, Inputting::ikC },
            { SDL_SCANCODE_D, Inputting::ikD },
            { SDL_SCANCODE_E, Inputting::ikE },
            { SDL_SCANCODE_F, Inputting::ikF },
            { SDL_SCANCODE_G, Inputting::ikG },
            { SDL_SCANCODE_H, Inputting::ikH },
            { SDL_SCANCODE_I, Inputting::ikI },
            { SDL_SCANCODE_J, Inputting::ikJ },
            { SDL_SCANCODE_K, Inputting::ikK },
            { SDL_SCANCODE_L, Inputting::ikL },
            { SDL_SCANCODE_M, Inputting::ikM },
            { SDL_SCANCODE_N, Inputting::ikN },
            { SDL_SCANCODE_O, Inputting::ikO },
            { SDL_SCANCODE_P, Inputting::ikP },
            { SDL_SCANCODE_Q, Inputting::ikQ },
            { SDL_SCANCODE_R, Inputting::ikR },
            { SDL_SCANCODE_S, Inputting::ikS },
            { SDL_SCANCODE_T, Inputting::ikT },
            { SDL_SCANCODE_U, Inputting::ikU },
            { SDL_SCANCODE_V, Inputting::ikV },
            { SDL_SCANCODE_W, Inputting::ikW },
            { SDL_SCANCODE_X, Inputting::ikX },
            { SDL_SCANCODE_Y, Inputting::ikY },
            { SDL_SCANCODE_Z, Inputting::ikZ },

            { SDL_SCANCODE_1, Inputting::ik1 },
            { SDL_SCANCODE_2, Inputting::ik2 },
            { SDL_SCANCODE_3, Inputting::ik3 },
            { SDL_SCANCODE_4, Inputting::ik4 },
            { SDL_SCANCODE_5, Inputting::ik5 },
            { SDL_SCANCODE_6, Inputting::ik6 },
            { SDL_SCANCODE_7, Inputting::ik7 },
            { SDL_SCANCODE_8, Inputting::ik8 },
            { SDL_SCANCODE_9, Inputting::ik9 },
            { SDL_SCANCODE_0, Inputting::ik0 },

            { SDL_SCANCODE_RETURN, Inputting::ikEnter },
            { SDL_SCANCODE_ESCAPE, Inputting::ikEscape },
            //            { SDL_SCANCODE_BACKSPACE, Inputting::ikUnknown },
            { SDL_SCANCODE_TAB, Inputting::ikTab },
            { SDL_SCANCODE_SPACE, Inputting::ikSpace },

            { SDL_SCANCODE_RIGHT, Inputting::ikRight },
            { SDL_SCANCODE_LEFT, Inputting::ikLeft },
            { SDL_SCANCODE_DOWN, Inputting::ikDown },
            { SDL_SCANCODE_UP, Inputting::ikUp },

            { SDL_SCANCODE_LCTRL, Inputting::ikCtrl },
            { SDL_SCANCODE_LSHIFT, Inputting::ikShift },
            { SDL_SCANCODE_LALT, Inputting::ikAlt }, /**< alt, option */
        //            { SDL_SCANCODE_LGUI,      Inputting::ikUnknown }, /**< windows, command (apple), meta */
        //            { SDL_SCANCODE_RCTRL,     Inputting::ikUnknown },
        //            { SDL_SCANCODE_RSHIFT,    Inputting::ikUnknown },
        //            { SDL_SCANCODE_RALT,      Inputting::ikUnknown }, /**< alt gr, option */
        //            { SDL_SCANCODE_RGUI,      Inputting::ikUnknown }, /**< windows, command (apple), meta */
        /* };*/

        InputtingWindow::InputtingWindow()
        {
            Windowing::Subscribe(this);
        }

        InputtingWindow::~InputtingWindow()
        {
            Windowing::UnSubscribe(this);
        }

        bool InputtingWindow::Init(const WindowSettings& settings, bool trapMouse_)
        {
            //_trapMouse = trapMouse_;
            return Window::Init(settings);
        }
        /*
        void InputtingWindow::SubscribeOnKeyboardEvents(Inputting::IKeyboardListener* listener)
        {
            ASSERT(listener)

            for (auto& item : _keyboardListeners)
            {
                if (listener == item)
                    throw Common::Exception("Error subscribe listener, listener already subscribed");
            }

            _keyboardListeners.push_back(listener);
        }

        void InputtingWindow::UnSubscribeOnKeyboardEvents(const Inputting::IKeyboardListener* listener)
        {
            ASSERT(listener)

            for (auto it = _keyboardListeners.begin(); it != _keyboardListeners.end();)
            {
                if (listener == *it)
                {
                    it = _keyboardListeners.erase(it);
                    return;
                }
                ++it;
            }
        }
        void InputtingWindow::SubscribeOnMouseEvents(Inputting::IMouseListener* listener)
        {
            ASSERT(listener)

            for (auto& item : _mouseListeners)
            {
                if (listener == item)
                    throw Common::Exception("Error subscribe listener, listener already subscribed");
            }

            _mouseListeners.push_back(listener);
        }

        void InputtingWindow::UnSubscribeOnMouseEvents(const Inputting::IMouseListener* listener)
        {
            ASSERT(listener)

            for (auto it = _mouseListeners.begin(); it != _mouseListeners.end();)
            {
                if (listener == *it)
                {
                    it = _mouseListeners.erase(it);
                    return;
                }
                ++it;
            }
        }

        void InputtingWindow::OnWindowFocusLost(const Window& window_)
        {
            if (&window_ != this)
                return;

            ReleaseMouse();
        }

        void InputtingWindow::OnKeyUp(const Window& window_, const SDL_Keysym& keysym)
        {
            if (&window_ != this)
                return;

            const auto& it = SDLToInputKeyMap.find(keysym.scancode);
            if (it == SDLToInputKeyMap.end())
                return;

            for (auto& listener : _keyboardListeners)
                listener->OnKeyUp(it->second);
        }

        void InputtingWindow::OnKeyDown(const Window& window_, const SDL_Keysym& keysym)
        {
            if (&window_ != this)
                return;

            const auto& it = SDLToInputKeyMap.find(keysym.scancode);
            if (it == SDLToInputKeyMap.end())
                return;

            for (auto& listener : _keyboardListeners)
                listener->OnKeyDown(it->second);
        }

        void InputtingWindow::OnMouseMotion(const Window& window_, const Vector2i& position, const Vector2i& relative)
        {
            if (&window_ != this)
                return;

            for (auto& listener : _mouseListeners)
                listener->OnMouseMove(position, relative);
        }

        void InputtingWindow::OnMouseButtonUp(const Window& window_, uint32_t button)
        {
            if (&window_ != this)
                return;

            Inputting::InputKey key;

            switch (button)
            {
            case SDL_BUTTON_LEFT:
                key = Inputting::ikMouseL;
                break;
            case SDL_BUTTON_MIDDLE:
                key = Inputting::ikMouseM;
                break;
            case SDL_BUTTON_RIGHT:
                key = Inputting::ikMouseR;
                break;
            case SDL_BUTTON_X1:
                return;
            case SDL_BUTTON_X2:
                return;
            default:
                return;
            }

            for (auto& listener : _mouseListeners)
                listener->OnButtonUp(key);
        }

        void InputtingWindow::OnMouseButtonDown(const Window& window_, uint32_t button)
        {
            if (&window_ != this)
                return;

            Inputting::InputKey key;

            switch (button)
            {
            case SDL_BUTTON_LEFT:
                key = Inputting::ikMouseL;
                if (_trapMouse && !_mouseHolded)
                    HoldMouse();
                break;
            case SDL_BUTTON_MIDDLE:
                key = Inputting::ikMouseM;
                break;
            case SDL_BUTTON_RIGHT:
                key = Inputting::ikMouseR;
                break;
            case SDL_BUTTON_X1:
                return;
            case SDL_BUTTON_X2:
                return;
            default:
                return;
            }

            for (auto& listener : _mouseListeners)
                listener->OnButtonDown(key);
        }

        void InputtingWindow::HoldMouse()
        {
            _mouseHolded = true;
            ShowCursor(false);
            SDL_SetWindowGrab(GetSDLWindow(), SDL_TRUE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }

        void InputtingWindow::ReleaseMouse()
        {
            _mouseHolded = false;
            ShowCursor(true);
            SDL_SetWindowGrab(GetSDLWindow(), SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }*/
    }
}