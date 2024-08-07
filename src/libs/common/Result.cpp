#include "Result.hpp"

namespace RR::Common
{
    std::string GetErrorMessage(RResult result)
    {
        switch (result)
        {
            case RResult::Ok: return u8"Operation successful";
            case RResult::False: return u8"No error occurred, but indicated false. E.g., "
                                        "indicates query data not ready yet for query data access";
            case RResult::Abort: return u8"Operation aborted";
            case RResult::FileNotFound: return u8"File/resource could not be found";
            case RResult::AccessDenied: return u8"General access denied error";
            case RResult::Fail: return u8"Unspecified failure";
            case RResult::InvalidHandle: return u8"Handle that is not valid";
            case RResult::InvalidArgument: return u8"One or more arguments are not valid";
            case RResult::NoInterface: return u8"No such interface supported";
            case RResult::NotImplemented: return u8"Not implemented";
            case RResult::OutOfMemory: return u8"Failed to allocate necessary memory";
            case RResult::DiskFull: return u8"There is not enough space on the disk";
            case RResult::Unexpected: return u8"Unexpected failure";
            case RResult::CannotOpen: return u8"File/Resource could not be opened";
            case RResult::NotFound: return u8"File/resource could not be found";
            case RResult::AlreadyExist: return u8"File/resource already exists";
            case RResult::InternalFail: return u8"An unhandled internal failure";
            case RResult::NotAvailable: return u8"Could not complete because some underlying feature (hardware or software) was not available";
            case RResult::ArithmeticOverflow: return u8"Arithmetic result exceeded range";
        }

        return fmt::sprintf("Unknown error. Code: 0x%08X", static_cast<typename std::underlying_type<RResult>::type>(result));
    }
}