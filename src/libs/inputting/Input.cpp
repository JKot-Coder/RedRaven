#include "Input.hpp"

#include "common/Math.hpp"

#include "windowing/Window.hpp"

namespace RR
{
    using namespace Common;

    namespace Inputting
    {
        std::unique_ptr<Input> Inputting::Input::_instance = std::unique_ptr<Input>(new Input());

        Input::~Input()
        {
            Terminate();
        }

        void Input::Reset()
        {
            memset(_down, 0, sizeof(_down));
            memset(&Mouse, 0, sizeof(Mouse));
        }

        void Input::Init()
        {
            Reset();
        }

        void Input::Terminate()
        {
            if (_inputtingWindow.get() != nullptr)
            {
                _inputtingWindow->UnSubscribeOnKeyboardEvents(this);
                _inputtingWindow->UnSubscribeOnMouseEvents(this);

                _inputtingWindow = nullptr;
            }
        }

        void Input::SubscribeToWindow(const std::shared_ptr<WindowSystem::InputtingWindow>& inputtingWindow_)
        {
            _inputtingWindow = inputtingWindow_;
            _inputtingWindow->SubscribeOnKeyboardEvents(this);
            _inputtingWindow->SubscribeOnMouseEvents(this);
        }

        void Input::Update()
        {
            Mouse.relative = Vector2i(0, 0);
        }

        void Input::SetDown(InputKey key, bool value)
        {
            if (_down[key] == value)
                return;

            // TOdo fix this
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

            _down[key] = value;

            if (value && key <= ikZ)
            {
                _lastKey = key;
            }
        }

        void Input::SetPos(InputKey key, const Vector2i& pos)
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

        void Input::OnMouseMove(const Vector2i& position, const Vector2i& relative)
        {
            Mouse.pos = position;
            Mouse.relative = relative;
        }

        void Input::OnButtonUp(InputKey inputKey)
        {
            SetDown(inputKey, false);
        }

        void Input::OnButtonDown(InputKey inputKey)
        {
            SetDown(inputKey, true);
        }
    }
}