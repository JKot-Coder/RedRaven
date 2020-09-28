#pragma once

namespace OpenDemo
{
    namespace Render
    {

#define D3DCall(exp, ...)                                                                                                                               \
    {                                                                                                                                                   \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "D3DCall takes only one argument use D3DCallCheck instead"); \
        Result result;                                                                                                                                  \
        if (ResultU::Failure(result = Result(exp)))                                                                                                     \
            return result;                                                                                                                              \
    }

#define D3DCallMsg(exp, msg, ...)                                                                                 \
    {                                                                                                             \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "Too many arguments"); \
        Result result;                                                                                            \
        if (ResultU::Failure(result = Result(exp)))                                                               \
        {                                                                                                         \
            LOG_ERROR("%s Code: 0x%08X Error: %s", msg, result, ResultU::ToString(result))                        \
            return result;                                                                                        \
        }                                                                                                         \
    }

        enum class Result : uint32_t
        {
            OK = 0,
            FALSE_CODE = 1,

            OUT_OF_MEMORY = 0x8007000E,

            FAIL = 0x80004005
        };

        namespace ResultU
        {
            U8String ToString(Result status);

            inline bool Success(Result status)
            {
                return static_cast<int32_t>(status) >= 0;
            }

            inline bool Failure(Result status)
            {
                return static_cast<int32_t>(status) < 0;
            }
        }
    }
}