#pragma once

namespace OpenDemo
{
    namespace Render
    {

#define D3DCall(exp, ...)                                                                                                                               \
    {                                                                                                                                                   \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "D3DCall takes only one argument use D3DCallCheck instead"); \
        GAPIResult result;                                                                                                                              \
        if (GAPIResultU::Failure(result = GAPIResult(exp)))                                                                                             \
            return result;                                                                                                                              \
    }

#define D3DCallMsg(exp, msg, ...)                                                                                 \
    {                                                                                                             \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "Too many arguments"); \
        GAPIResult result;                                                                                        \
        if (GAPIResultU::Failure(result = GAPIResult(exp)))                                                       \
        {                                                                                                         \
            LOG_ERROR("%s Code: 0x%08X Error: %s", msg, result, GAPIResultU::ToString(result))                    \
            return result;                                                                                        \
        }                                                                                                         \
    }

        enum class GAPIResult : uint32_t
        {
            OK = 0,
            FALSE_CODE = 1,

            OUT_OF_MEMORY = 0x8007000E,

            FAIL = 0x80004005
        };

        namespace GAPIResultU
        {
            U8String ToString(GAPIResult status);

            inline bool Success(GAPIResult status)
            {
                return static_cast<int32_t>(status) >= 0;
            }

            inline bool Failure(GAPIResult status)
            {
                return static_cast<int32_t>(status) < 0;
            }
        }
    }
}