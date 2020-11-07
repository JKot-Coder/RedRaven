#include "Result.hpp"

namespace OpenDemo
{
    namespace Render
    {
        U8String Result::ToString()
        {
            switch (value_)
            {
            case Result::OK:
                return u8"No error occurred.";
            case Result::FALSE_CODE:
                return u8"No error occurred, but indicated false. E.g., "
                       "indicates query data not ready yet for query data access.";
            case Result::FAIL:
                return u8"An undetermined error occurred";
            }

            Log::Print::Error("Unknown Result : %d", value_);
            return "Unknown error. ";
        }
    }
}