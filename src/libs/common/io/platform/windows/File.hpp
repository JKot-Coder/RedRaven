#pragma once

#include <wtypes.h>

namespace RR::Common::IO
{
    class FileData
    {
    protected:
        static inline HANDLE InvalidHandle = INVALID_HANDLE_VALUE;
        HANDLE handle_ = InvalidHandle;
    };
}
