#include "GAPIStatus.hpp"

#include "common/Logger.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace GAPIStatusU
        {
            U8String ToString(GAPIStatus status)
            {
                switch (status)
                {
                case GAPIStatus::OK:
                    return u8"No error occurred.";
                case GAPIStatus::FALSE_CODE:
                    return u8"No error occurred, but indicated false. E.g., "
                           "indicates query data not ready yet for query data access.";
                case GAPIStatus::FAIL:
                    return u8"An undetermined error occurred";
                }

                Log::Print::Error("Unknown GAPIStatus : %d", status);
                return "Unknown error. ";
            }
        }
    }
}