#include <string>

namespace RR::Rfx
{
    enum class RfxResult : int32_t;

    std::string GetErrorMessage(RfxResult result);
}