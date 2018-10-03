#ifndef OPENDEMO_INPUT_H
#define OPENDEMO_INPUT_H

#include "core.h"


namespace Input {

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

    InputKey lastKey;
    bool down[ikMAX];

    struct Mouse {
        vec2 pos;
        struct {
            vec2 L, R, M;
        } start;
    } mouse;

    void setDown(InputKey key, bool value, int index = 0) {
        if (down[key] == value)
            return;

        if (value)
            switch (key) {
                case ikMouseL:
                    mouse.start.L = mouse.pos;
                    break;
                case ikMouseR:
                    mouse.start.R = mouse.pos;
                    break;
                case ikMouseM:
                    mouse.start.M = mouse.pos;
                    break;
            }
        down[key] = value;

        if (value && key <= ikZ) {
            lastKey = key;
        }
    }

    void
    setPos(InputKey key, const vec2 &pos) {
        switch (key) {
            case ikMouseL:
            case ikMouseR:
            case ikMouseM:
                mouse.pos = pos;
                return;
            default:
                return;
        }
    }

    void reset() {
        memset(down, 0, sizeof(down));
        memset(&mouse, 0, sizeof(mouse));
    }

    void init() {
        reset();
    }

    void update() {
    }

}
#endif //OPENDEMO_INPUT_H
