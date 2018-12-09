#pragma once

#include <cstdint>

namespace Common {

    class Stream {
    public:
        virtual ~Stream() {};

        virtual int GetPosition() = 0;
        virtual void SetPosition(int value) = 0;

        virtual int GetSize() = 0;

        virtual int Read(char *data, int length) = 0;
        virtual int Write(const char *data, int length) = 0;
    };

}