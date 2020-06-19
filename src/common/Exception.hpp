#pragma once

#include "common/String.hpp"

#include <exception>

namespace OpenDemo
{
    namespace Common
    {
        class Exception : public std::exception
        {
        public:
            Exception(const char* fmt, ...);
            virtual ~Exception() throw();

            inline virtual const char* what() const throw()
            {
                return message.c_str();
            }

        private:
            U8String message;
        };
    }
}