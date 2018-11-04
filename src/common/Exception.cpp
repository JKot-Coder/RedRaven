#include <cstdio>
#include <stdarg.h>

#include "common/Exception.hpp"

namespace Common {

    Exception::Exception(const std::string& fmt, ...) {
        va_list args;
        size_t size_buffer = (size_t)fmt.size() * 2;
        std::string str;

        while (true) {
            str.resize(size_buffer);

            va_start(args, fmt);
            int size_out = vsnprintf((char *) str.data(), size_buffer, fmt.c_str(), args);
            va_end(args);

            // see http://perfec.to/vsnprintf/pasprintf.c
            // if size_out ...
            //      == -1             --> output was truncated
            //      == size_buffer    --> output was truncated
            //      == size_buffer-1  --> ambiguous, /may/ have been truncated
            //       > size_buffer    --> output was truncated, and size_out
            //                            bytes would have been written
            if (size_out == size_buffer || size_out == -1 || size_out == size_buffer - 1)
                size_buffer *= 2;
            else if (size_out > size_buffer)
                size_buffer = size_out + 2; // to avoid the ambiguous case
            else
                break;
        }
        message = str;
    }

    Exception::~Exception() throw() {
    }

}