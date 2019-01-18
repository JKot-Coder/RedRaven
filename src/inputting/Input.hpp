#pragma once

#include <map>

#include "common/VecMath.h"
#include "common/Utils.hpp"

#include "windowing/Windowing.hpp"

namespace Inputting {

    enum InputKey { ikNone,
        // keyboard
        ikLeft, ikRight, ikUp, ikDown, ikSpace, ikTab, ikEnter, ikEscape, ikShift, ikCtrl, ikAlt,
        ik0, ik1, ik2, ik3, ik4, ik5, ik6, ik7, ik8, ik9,
        ikA, ikB, ikC, ikD, ikE, ikF, ikG, ikH, ikI, ikJ, ikK, ikL, ikM,
        ikN, ikO, ikP, ikQ, ikR, ikS, ikT, ikU, ikV, ikW, ikX, ikY, ikZ,
        // mouse
        ikMouseL, ikMouseR, ikMouseM,
        ikMAX
    };

    class Input final : Windowing::Windowing::Listener {
    public:

        struct Mouse {
            Common::vec2 pos;
            Common::vec2 relative;
            struct {
                Common::vec2 L, R, M;
            } start;
        } Mouse;

        ~Input();

        void Reset();
        void Init();
        void Update();

        inline bool IsDown(InputKey inputKey) const {
            ASSERT(inputKey >= 0 && inputKey < ikMAX )

            return down[inputKey];
        }

        inline static const std::unique_ptr<Input>& Instance() {
            return instance;
        }

        void TrapMouseInWindow(const std::shared_ptr<Windowing::Window> &window);

    private:
        std::shared_ptr<Windowing::Window> mouseWindowTrap;

        InputKey lastKey;
        bool down[ikMAX];

        static std::unique_ptr<Input> instance;

        virtual void OnKeyUp(const SDL_Keysym &keysym) override;
        virtual void OnKeyDown(const SDL_Keysym &keysym) override;

        virtual void OnMouseMotion(const SDL_MouseMotionEvent &mouseMotionEvent) override;

        void SetDown(InputKey key, bool value);
        void SetPos(InputKey key, const Common::vec2 &pos);
    };

    inline static const std::unique_ptr<Input>& Instance() {
        return Input::Instance();
    }

}