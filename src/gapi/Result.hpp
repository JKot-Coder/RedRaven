#pragma once

namespace OpenDemo
{
    namespace Render
    {

#define D3DCall(exp, ...)                                                                                                                               \
    {                                                                                                                                                   \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "D3DCall takes only one argument use D3DCallCheck instead"); \
        Result result = Result(exp);                                                                                                                    \
        if (!result)                                                                                                                                    \
            return result;                                                                                                                              \
    }

#define D3DCallMsg(exp, msg, ...)                                                                                 \
    {                                                                                                             \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "Too many arguments"); \
        Result result = Result(exp);                                                                              \
        if (!result)                                                                                              \
        {                                                                                                         \
            LOG_ERROR("%s Code: 0x%08X Error: %s", msg, result, result.ToString())                                \
            return result;                                                                                        \
        }                                                                                                         \
    }

        class Result
        {
        public:
            enum Value : uint32_t
            {
                OK = 0,
                FALSE_CODE = 1,

                OUT_OF_MEMORY = 0x8007000E,

                FAIL = 0x80004005
            };

            Result() = default;

            constexpr Result(uint32_t value) : value_(static_cast<Value>(value))
            {
            }

            constexpr Result(Value value) : value_(value)
            {
            }

            operator Value() const
            {
                return value_;
            } // Allow switch and comparisons.
            // note: Putting constexpr here causes
            // clang to stop warning on incomplete
            // case handling.

            explicit operator bool() const
            {
                return static_cast<int32_t>(value_) >= 0;
            }

        public:
            U8String ToString();

            inline bool IsSuccess() const { return static_cast<int32_t>(value_) >= 0; }
            inline bool IsFailure() const { return static_cast<int32_t>(value_) < 0; }

        private:
            Value value_;
        };
    }
}