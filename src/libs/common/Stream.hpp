#pragma once

#include <istream>

namespace RR
{
    namespace Common
    {
        class Stream
        {
        public:
            virtual ~Stream() {};

            virtual std::string GetName() const = 0;

            virtual int64_t GetPosition() = 0;
            virtual void SetPosition(int64_t value) = 0;

            virtual int64_t GetSize() = 0;

            virtual int64_t Read(char* data, int64_t length) = 0;
            virtual int64_t Write(const char* data, int64_t length) = 0;

            virtual std::istream* GetNativeStream() = 0;
        };
    }
}