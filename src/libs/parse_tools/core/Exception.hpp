#pragma once

#include <stdexcept>

namespace RR
{
    namespace ParseTools
    {
        // Exceptions should not generally be used in code, use the 'signal' mechanism
        // ideally using the macros in the Signal.hpp such as `PARSE_UNEXPECTED`

        using Exception = std::runtime_error;

        class InvalidOperationException : public Exception
        {
        public:
            InvalidOperationException() = delete;
            InvalidOperationException(const std::string& message)
                : Exception(message)
            {
            }
        };

        class InternalError : public Exception
        {
        public:
            InternalError() = delete;
            InternalError(const std::string& message)
                : Exception(message)
            {
            }
        };

        class AbortCompilationException : public Exception
        {
        public:
            AbortCompilationException() = delete;
            AbortCompilationException(const std::string& message)
                : Exception(message)
            {
            }
        };
    }
}