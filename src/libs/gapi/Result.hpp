#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        struct Result final
        {
        public:
            // TODO dangerous. auto result = Result::OK; result type is uint32_t
            enum Value : uint32_t
            {
                Ok = 0,
                False = 1,

                Abort = 0x80004004,
                AccessDenied = 0x80070005,
                Fail = 0x80004005,
                Handle = 0x80070006,
                InvalidArgument = 0x80070057,
                NoInterface = 0x80004002,
                NotImplemented = 0x80004001,
                OutOfMemory = 0x8007000E,
                Pointer = 0x80004003,
                Unexpected = 0x8000FFFF,

                DeviceRemoved = 0x887A0005,
                DeviceReset = 0x887A0007,

                WaitFailed = 0xFFFFFFFF,
                WaitAbandoned = 0x00000080,
                WaitTimeout = 0x00000102,
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

            explicit operator bool() const
            {
                return static_cast<int32_t>(value_) >= 0;
            }

        public:
            std::string ToString() const;

            inline bool IsSuccess() const { return static_cast<int32_t>(value_) >= 0; }
            inline bool IsFailure() const { return static_cast<int32_t>(value_) < 0; }

        private:
            Value value_;
        };
    }
}