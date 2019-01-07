#pragma once

#include <cstdint>
#include <istream>

namespace Common {

    class Stream {
    public:
        virtual ~Stream() {};

        virtual std::string GetName() const = 0;

        virtual int GetPosition() = 0;
        virtual void SetPosition(int value) = 0;

        virtual int GetSize() = 0;

        virtual int Read(char *data, int length) = 0;
        virtual int Write(const char *data, int length) = 0;

        virtual std::istream* GetNativeStream() = 0;
    };

}