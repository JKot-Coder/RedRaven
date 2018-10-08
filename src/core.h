#ifndef OPENDEMO_CORE_H
#define OPENDEMO_CORE_H

#include <cstring>
#include "vecmath.h"
#include "input.h"

#ifdef RENDER_GL
  #include "render_gl.h"
#endif

namespace Core {
    bool isQuit;
    int  x, y, width, height;

    void quit() {
        isQuit = true;
    }
}

#endif //OPENDEMO_CORE_H

