#pragma once

#include <stdexcept>

namespace RR
{
    namespace Rfx
    {
        // Exceptions should not generally be used in core/compiler-core, use the 'signal' mechanism
        // ideally using the macros in the signal.hpp such as `RFX_UNEXPECTED`

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