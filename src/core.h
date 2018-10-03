#ifndef OPENDEMO_CORE_H
#define OPENDEMO_CORE_H

#include <cstring>
#include "vecmath.h"
#include "input.h"

#ifdef GAPI_GL
    #include "gl.h"
#endif

namespace Core {
    bool isQuit;
    int  x, y, width, height;

    void quit() {
        isQuit = true;
    }
}

#endif //OPENDEMO_CORE_H

