#pragma once

#include <cstdint>

namespace Common {

    class Stream {
    public:
        virtual int32_t GetPosition() = 0;
        virtual void SetPosition(int32_t value) = 0;
        virtual int32_t Read(char *data, int32_t length) = 0;
        virtual int32_t Write(const char *data, int32_t length) = 0;
    };

}