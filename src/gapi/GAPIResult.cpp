#include "GAPIResult.hpp"

#include "common/Logger.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace GAPIResultU
        {
            U8String ToString(GAPIResult status)
            {
                switch (status)
                {
                case GAPIResult::OK:
                    return u8"No error occurred.";
                case GAPIResult::FALSE_CODE:
                    return u8"No error occurred, but indicated false. E.g., "
                           "indicates query data not ready yet for query data access.";
                case GAPIResult::FAIL:
                    return u8"An undetermined error occurred";
                }

                Log::Print::Error("Unknown GAPIResult : %d", status);
                return "Unknown error. ";
            }
        }
    }
}