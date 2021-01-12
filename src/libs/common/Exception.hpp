#pragma once

#include <exception>

namespace OpenDemo
{
    namespace Common
    {
        class Exception : public std::exception
        {
        public:
            Exception::Exception(const U8String& msg)
                : _message(msg)
            {
            }
            inline const char* what() const override
            {
                return _message.c_str();
            }

        private:
            U8String _message;
        };
    }
}