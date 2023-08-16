#include "ErrorNo.hpp"

namespace RR::Common
{
    std::string StrError(int errnum)
    {
        std::string str;
        if (errnum == 0)
            return str;

        const int maxErrorStringLength = 2000;
        char buffer[maxErrorStringLength];
        buffer[0] = '\0';

#ifdef _WIN32
        strerror_s(buffer, maxErrorStringLength - 1, errnum);
        str = buffer;
#else
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
        // glibc defines its own incompatible version of strerror_r
        // which may not use the buffer supplied.
        str = strerror_r(errnum, buffer, maxErrorStringLength - 1);
#else
        strerror_r(errnum, buffer, maxErrorStringLength - 1);
        str = buffer;
#endif
#endif

#if 0
        // Copy the thread un-safe result of strerror into
        // the buffer as fast as possible to minimize impact
        // of collision of strerror in multiple threads.
        str = strerror(errnum);
#endif
        return str;
    }
}