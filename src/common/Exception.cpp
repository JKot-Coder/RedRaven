#include <cstdio>
#include <stdarg.h>

#include "common/Exception.hpp"

namespace Common {

    Exception::Exception(const char *fmt, ...) {
        va_list args;
        size_t sizeBuffer = strlen(fmt) * 2;
        std::string str;

        while (true) {
            str.resize(sizeBuffer);

            va_start(args, fmt);
            int sizeOut = vsnprintf((char *) str.data(), sizeBuffer, fmt, args);
            va_end(args);

            // see http://perfec.to/vsnprintf/pasprintf.c
            // if sizeOut ...
            //      == -1             --> output was truncated
            //      == sizeBuffer    --> output was truncated
            //      == sizeBuffer-1  --> ambiguous, /may/ have been truncated
            //       > sizeBuffer    --> output was truncated, and sizeOut
            //                            bytes would have been written
            if (sizeOut == static_cast<int>(sizeBuffer) || sizeOut == -1 || sizeOut == static_cast<int>(sizeBuffer) - 1)
                sizeBuffer *= 2;
            else if (sizeOut > static_cast<int>(sizeBuffer))
                sizeBuffer = sizeOut + 2; // to avoid the ambiguous case
            else
                break;
        }
        message = str;
    }

    Exception::~Exception() throw() {
    }

}