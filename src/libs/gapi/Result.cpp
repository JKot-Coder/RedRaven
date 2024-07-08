#include "Result.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        std::string Result::ToString() const
        {
            switch (value_)
            {
                case Result::Ok:
                    return u8"Operation successful";
                case Result::False:
                    return u8"No error occurred, but indicated false. E.g., "
                           "indicates query data not ready yet for query data access";
                case Result::Abort:
                    return u8"Operation aborted";
                case Result::AccessDenied:
                    return u8"General access denied error";
                case Result::Fail:
                    return u8"Unspecified failure";
                case Result::Handle:
                    return u8"Handle that is not valid";
                case Result::InvalidArgument:
                    return u8"One or more arguments are not valid";
                case Result::NoInterface:
                    return u8"No such interface supported";
                case Result::NotImplemented:
                    return u8"Not implemented";
                case Result::OutOfMemory:
                    return u8"Failed to allocate necessary memory";
                case Result::Pointer:
                    return u8"Pointer that is not valid";
                case Result::Unexpected:
                    return u8"Unexpected failure";

                case Result::DeviceRemoved:
                    return u8"Device was removed";
                case Result::DeviceReset:
                    return u8"Device was reset";

                case Result::WaitFailed:
                    return u8"The function has failed.";
                case Result::WaitAbandoned:
                    return u8"The specified object is a mutex object that was not released by the thread that owned the mutex object before the owning thread terminated";
                case Result::WaitTimeout:
                    return u8"The time-out interval elapsed";
            }

            return fmt::sprintf("Unknown error. Code: 0x%08X", value_);
        }
    }
}