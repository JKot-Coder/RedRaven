#pragma once

#include <map>

#include "common/Math.hpp"

#include "windowing/InputtingWindow.hpp"

namespace OpenDemo
{
    namespace Inputting
    {

        enum InputKey
        {
            ikNone,
            // keyboard
            ikLeft,
            ikRight,
            ikUp,
            ikDown,
            ikSpace,
            ikTab,
            ikEnter,
            ikEscape,
            ikShift,
            ikCtrl,
            ikAlt,
            ik0,
            ik1,
            ik2,
            ik3,
            ik4,
            ik5,
            ik6,
            ik7,
            ik8,
            ik9,
            ikA,
            ikB,
            ikC,
            ikD,
            ikE,
            ikF,
            ikG,
            ikH,
            ikI,
            ikJ,
            ikK,
            ikL,
            ikM,
            ikN,
            ikO,
            ikP,
            ikQ,
            ikR,
            ikS,
            ikT,
            ikU,
            ikV,
            ikW,
            ikX,
            ikY,
            ikZ,
            // mouse
            ikMouseL,
            ikMouseR,
            ikMouseM,
            ikMAX
        };

        class IKeyboardListener
        {
        public:
            virtual void OnKeyUp(InputKey inputKey) { (void)inputKey; }
            virtual void OnKeyDown(InputKey inputKey) { (void)inputKey; }
        };

        class IMouseListener
        {
        public:
            virtual void OnMouseMove(const Vector2i& position, const Vector2i& relative)
            {
                (void)position;
                (void)relative;
            }
            virtual void OnButtonUp(InputKey inputKey) { (void)inputKey; }
            virtual void OnButtonDown(InputKey inputKey) { (void)inputKey; }
        };

        class Input final : public IMouseListener, IKeyboardListener
        {
        public:
            struct Mouse
            {
                Vector2i pos;
                Vector2i relative;
                struct
                {
                    Vector2i L, R, M;
                } start;
            } Mouse;

            ~Input();

            void Reset();
            void Init();
            void Terminate();
            void Update();

            inline bool IsDown(InputKey inputKey) const
            {
                ASSERT(inputKey >= 0 && inputKey < ikMAX)

                return _down[inputKey];
            }

            inline static const std::unique_ptr<Input>& Instance()
            {
                return _instance;
            }

            void SubscribeToWindow(const std::shared_ptr<Windowing::InputtingWindow>& inputtingWindow);

        private:
            static std::unique_ptr<Input> _instance;

            std::shared_ptr<Windowing::InputtingWindow> _inputtingWindow;

            InputKey _lastKey;
            bool _down[ikMAX];

            virtual void OnKeyUp(InputKey inputKey) override;
            virtual void OnKeyDown(InputKey inputKey) override;

            virtual void OnMouseMove(const Vector2i& position, const Vector2i& relative) override;
            virtual void OnButtonUp(InputKey inputKey) override;
            virtual void OnButtonDown(InputKey inputKey) override;

            void SetDown(InputKey key, bool value);
            void SetPos(InputKey key, const Vector2i& pos);
        };

        inline static const std::unique_ptr<Input>& Instance()
        {
            return Input::Instance();
        }
    }
}