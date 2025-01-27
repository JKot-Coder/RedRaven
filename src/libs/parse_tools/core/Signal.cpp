#include "Signal.hpp"

#include "rfx/core/Exception.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            std::string getSignalTypeAsText(SignalType type)
            {
                switch (type)
                {
                    case SignalType::AssertFailure: return "assert failure";
                    case SignalType::Unimplemented: return "unimplemented";
                    case SignalType::Unreachable: return "hit unreachable code";
                    case SignalType::Unexpected: return "unexpected";
                    case SignalType::InvalidOperation: return "invalid operation";
                    case SignalType::AbortCompilation: return "abort compilation";
                    default: return "unhandled";
                }
            }
        }

        // One point of having as a single function is a choke point both for handling (allowing different
        // handling scenarios) as well as a choke point to set a breakpoint to catch 'signal' types
        void handleSignal(SignalType type, const std::string& message)
        {
            const auto& formatedMsg = fmt::format("{0}: {1}", getSignalTypeAsText(type), message);

            Log::Print::Error(formatedMsg);

            switch (type)
            {
                case SignalType::InvalidOperation: throw InvalidOperationException(formatedMsg);
                case SignalType::AbortCompilation: throw AbortCompilationException(formatedMsg);
                default: throw InternalError(formatedMsg);
            }
        }
    }
}