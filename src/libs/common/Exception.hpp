#pragma once

#include <exception>

namespace RR
{
    namespace Common
    {
        class Exception : public std::exception
        {
        public:
            Exception::Exception(const std::string& msg)
                : _message(msg)
            {
            }
            inline const char* what() const override
            {
                return _message.c_str();
            }

        private:
            std::string _message;
        };
    }
}