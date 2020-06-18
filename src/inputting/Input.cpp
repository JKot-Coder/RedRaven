#include "Input.hpp"

#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

#include "common/Common.hpp"
#include "common/VecMath.h"

#include "windowing/Window.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace Inputting
    {
        std::unique_ptr<Input> Inputting::Input::instance = std::unique_ptr<Input>(new Input());

        Input::~Input()
        {
            Terminate();
        }

        void Input::Reset()
        {
            memset(down, 0, sizeof(down));
            memset(&Mouse, 0, sizeof(Mouse));
        }

        void Input::Init()
        {
            Reset();
        }

        void Input::Terminate()
        {
            if (inputtingWindow.get() != nullptr)
            {
                inputtingWindow->UnSubscribeOnKeyboardEvents(this);
                inputtingWindow->UnSubscribeOnMouseEvents(this);

                inputtingWindow = nullptr;
            }
        }

        void Input::SubscribeToWindow(const std::shared_ptr<Windowing::InputtingWindow>& inputtingWindow_)
        {
            inputtingWindow = inputtingWindow_;
            inputtingWindow->SubscribeOnKeyboardEvents(this);
            inputtingWindow->SubscribeOnMouseEvents(this);
        }

        void Input::Update()
        {
            Mouse.relative = vec2(0, 0);
        }

        void Input::SetDown(InputKey key, bool value)
        {
            if (down[key] == value)
                return;

            //TOdo fix this
            if (value)
                switch (key)
                {
                case ikMouseL:
                    //                   mouse.start.L = mouse.pos;
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

            if (value && key <= ikZ)
            {
                lastKey = key;
            }
        }

        void Input::SetPos(InputKey key, const vec2& pos)
        {
            (void)pos;
            switch (key)
            {
            case ikMouseL:
            case ikMouseR:
            case ikMouseM:
                //     mouse.pos = pos;
                return;
            default:
                return;
            }
        }

        void Input::OnKeyUp(InputKey inputKey)
        {
            SetDown(inputKey, false);
        }

        void Input::OnKeyDown(InputKey inputKey)
        {
            SetDown(inputKey, true);
        }

        void Input::OnMouseMove(const Common::vec2& position, const Common::vec2& relative)
        {
            Mouse.pos = position;
            Mouse.relative = relative;
        }

        void Input::OnButtonUp(InputKey inputKey)
        {
            ;
            SetDown(inputKey, false);
        }

        void Input::OnButtonDown(InputKey inputKey)
        {
            SetDown(inputKey, true);
        }
    }
}