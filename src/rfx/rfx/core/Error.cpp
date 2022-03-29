#include "Error.hpp"

#include "include/rfx.hpp"

namespace RR::Rfx
{
    std::string GetErrorMessage(RfxResult result)
    {
        switch (result)
        {
            case RfxResult::Ok:
                return u8"Operation successful";
                // case RfxResult::False:
                //     return u8"No error occurred, but indicated false. E.g., "
                //           "indicates query data not ready yet for query data access";
            case RfxResult::Abort:
                return u8"Operation aborted";
            case RfxResult::AccessDenied:
                return u8"General access denied error";
            case RfxResult::Fail:
                return u8"Unspecified failure";
            case RfxResult::InvalidHandle:
                return u8"Handle that is not valid";
            case RfxResult::InvalidArgument:
                return u8"One or more arguments are not valid";
            case RfxResult::NoInterface:
                return u8"No such interface supported";
            case RfxResult::NotImplemented:
                return u8"Not implemented";
            case RfxResult::OutOfMemory:
                return u8"Failed to allocate necessary memory";
            case RfxResult::Unexpected:
                return u8"Unexpected failure";

            case RfxResult::CannotOpen:
                return u8"File/Resource could not be opened";
            case RfxResult::NotFound:
                return u8"File/resource could not be found";
            case RfxResult::InternalFail:
                return u8"An unhandled internal failure";
            case RfxResult::NotAvailable:
                return u8"Could not complete because some underlying feature (hardware or software) was not available";
        }

        return fmt::sprintf("Unknown error. Code: 0x%08X", static_cast<typename std::underlying_type<RfxResult>::type>(result));
    }
}