#pragma once

#include <exception>
#include <string>

namespace Common {

    class Exception: public std::exception {
    public:
        Exception(const char* fmt, ...);
        virtual ~Exception() throw();

        inline virtual const char *what() const throw()
        {
            return message.c_str();
        }

    private:
        std::string message;
    };

}