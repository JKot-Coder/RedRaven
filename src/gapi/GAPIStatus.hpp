#pragma once

namespace OpenDemo
{
    namespace Render
    {

        enum class GAPIStatus : uint32_t
        {
            OK = 0,
            FALSE_CODE = 1,

            FAIL = 0x80004005
        };

        namespace GAPIStatusU
        {
            U8String ToString(GAPIStatus status);

            inline bool Success(GAPIStatus status)
            {
                return static_cast<int32_t>(status) >= 0;
            }

            inline bool Failure(GAPIStatus status)
            {
                return static_cast<int32_t>(status) < 0;
            }
        }

    }
}