#pragma once

namespace RR
{
    namespace ParseTools
    {
        enum class SignalType
        {
            Unexpected,
            Unimplemented,
            AssertFailure,
            Unreachable,
            InvalidOperation,
            AbortCompilation,
        };

        void handleSignal(SignalType type, const std::string& message);
        // clang-format off
        #define PARSE_UNEXPECTED(reason) \
            handleSignal(SignalType::Unexpected, reason)

        #define PARSE_UNIMPLEMENTED_X(what) \
            handleSignal(SignalType::Unimplemented, what)

        #define PARSE_UNREACHABLE(msg) \
            handleSignal(SignalType::Unreachable, msg)

        //TODO ?
        #define PARSE_ASSERT_FAILURE(msg) \
            handleSignal(SignalType::AssertFailure, msg)

        #define PARSE_INVALID_OPERATION(msg) \
            handleSignal(SignalType::InvalidOperation, msg)

        #define PARSE_ABORT(msg) \
            handleSignal(SignalType::AbortCompilation, msg)
        // clang-format on
    }
}