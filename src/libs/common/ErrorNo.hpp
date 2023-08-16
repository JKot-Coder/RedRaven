#include <errno.h>

namespace RR::Common
{
    std::string StrError(int errnum);
    inline std::string GetErrorMessage()  { return StrError(errno); };
}