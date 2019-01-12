#include <SDL_scancode.h>
#include <SDL_keyboard.h>
#include <SDL_events.h>

#include "common/Utils.hpp"
#include "common/VecMath.h"

#include "windowing/Window.hpp"

#include "inputting/Input.hpp"

using namespace Common;

namespace Inputting {

    std::unique_ptr<Input> Inputting::Input::instance = std::unique_ptr<Input>(new Input());

    static const std::map<SDL_Scancode, InputKey> SDLToInputKeyMap = {

            { SDL_SCANCODE_A, ikA },
            { SDL_SCANCODE_B, ikB },
            { SDL_SCANCODE_C, ikC },
            { SDL_SCANCODE_D, ikD },
            { SDL_SCANCODE_E, ikE },
            { SDL_SCANCODE_F, ikF },
            { SDL_SCANCODE_G, ikG },
            { SDL_SCANCODE_H, ikH },
            { SDL_SCANCODE_I, ikI },
            { SDL_SCANCODE_J, ikJ },
            { SDL_SCANCODE_K, ikK },
            { SDL_SCANCODE_L, ikL },
            { SDL_SCANCODE_M, ikM },
            { SDL_SCANCODE_N, ikN },
            { SDL_SCANCODE_O, ikO },
            { SDL_SCANCODE_P, ikP },
            { SDL_SCANCODE_Q, ikQ },
            { SDL_SCANCODE_R, ikR },
            { SDL_SCANCODE_S, ikS },
            { SDL_SCANCODE_T, ikT },
            { SDL_SCANCODE_U, ikU },
            { SDL_SCANCODE_V, ikV },
            { SDL_SCANCODE_W, ikW },
            { SDL_SCANCODE_X, ikX },
            { SDL_SCANCODE_Y, ikY },
            { SDL_SCANCODE_Z, ikZ },

            { SDL_SCANCODE_1, ik1 },
            { SDL_SCANCODE_2, ik2 },
            { SDL_SCANCODE_3, ik3 },
            { SDL_SCANCODE_4, ik4 },
            { SDL_SCANCODE_5, ik5 },
            { SDL_SCANCODE_6, ik6 },
            { SDL_SCANCODE_7, ik7 },
            { SDL_SCANCODE_8, ik8 },
            { SDL_SCANCODE_9, ik9 },
            { SDL_SCANCODE_0, ik0 },

            { SDL_SCANCODE_RETURN,    ikEnter   },
            { SDL_SCANCODE_ESCAPE,    ikEscape  },
//            { SDL_SCANCODE_BACKSPACE, ikUnknown },
            { SDL_SCANCODE_TAB,       ikTab     },
            { SDL_SCANCODE_SPACE,     ikSpace   },

            { SDL_SCANCODE_RIGHT,     ikRight   },
            { SDL_SCANCODE_LEFT,      ikLeft    },
            { SDL_SCANCODE_DOWN,      ikDown    },
            { SDL_SCANCODE_UP,        ikUp      },

            { SDL_SCANCODE_LCTRL,     ikCtrl    },
            { SDL_SCANCODE_LSHIFT,    ikShift   },
            { SDL_SCANCODE_LALT,      ikAlt     }, /**< alt, option */
//            { SDL_SCANCODE_LGUI,      ikUnknown }, /**< windows, command (apple), meta */
//            { SDL_SCANCODE_RCTRL,     ikUnknown },
//            { SDL_SCANCODE_RSHIFT,    ikUnknown },
//            { SDL_SCANCODE_RALT,      ikUnknown }, /**< alt gr, option */
//            { SDL_SCANCODE_RGUI,      ikUnknown }, /**< windows, command (apple), meta */

    };

    Input::~Input() {
        Windowing::Windowing::UnSubscribe(this);
    }

    void Input::Reset() {
        memset(down, 0, sizeof(down));
        memset(&Mouse, 0, sizeof(Mouse));
    }

    void Input::Init() {
        Reset();

        Windowing::Windowing::Subscribe(this);
    }

    void Input::Update() {
        if(!mouseWindowTrap.get())
            return;

        if (SDL_GetMouseFocus() == mouseWindowTrap->GetSDLWindow()) {
            int height = mouseWindowTrap->GetHeight();
            int width = mouseWindowTrap->GetWidth();
            mouseWindowTrap->SetMousePos(width / 2, height / 2);
            mouseWindowTrap->ShowCursor(false);
        }

        Mouse.relative = vec2(0, 0);
    }

    void Input::SetDown(InputKey key, bool value) {
        if (down[key] == value)
            return;

        //TOdo fix this
        if (value)
            switch (key) {
                case ikMouseL:
//                    mouse.start.L = mouse.pos;
                    break;
                case ikMouseR:
  //                  mouse.start.R = mouse.pos;
                    break;
                case ikMouseM:
    //                mouse.start.M = mouse.pos;
                    break;
                default:
                    break;
            }

        down[key] = value;

        if (value && key <= ikZ) {
            lastKey = key;
        }
    }

    void Input::SetPos(InputKey key, const vec2 &pos) {
        (void) pos;
        switch (key) {
            case ikMouseL:
            case ikMouseR:
            case ikMouseM:
           //     mouse.pos = pos;
                return;
            default:
                return;
        }
    }

    void Input::KeyUp(const SDL_Keysym &keysym) {
        const auto &it = SDLToInputKeyMap.find(keysym.scancode);
        if(it == SDLToInputKeyMap.end())
            return;

        SetDown(it->second, false);
    }

    void Input::KeyDown(const SDL_Keysym &keysym) {
        const auto &it = SDLToInputKeyMap.find(keysym.scancode);
        if(it == SDLToInputKeyMap.end())
            return;

        SetDown(it->second, true);
    }

    void Input::MouseMotion(const SDL_MouseMotionEvent &mouseMotionEvent) {
        if (mouseWindowTrap.get()) {
            int height = mouseWindowTrap->GetHeight();
            int width = mouseWindowTrap->GetWidth();
            vec2 center = vec2(width, height) * 0.5f;
            vec2 mouse = vec2(mouseMotionEvent.x, mouseMotionEvent.y);
            Mouse.relative = mouse - center;
        }
        Mouse.pos = vec2(mouseMotionEvent.x, mouseMotionEvent.y);
    }

    void Input::TrapMouseInWindow(const std::shared_ptr<Windowing::Window> &window) {
        ASSERT(window.get())

        mouseWindowTrap = window;
        SDL_SetWindowGrab(mouseWindowTrap->GetSDLWindow(), SDL_TRUE);
    }

}