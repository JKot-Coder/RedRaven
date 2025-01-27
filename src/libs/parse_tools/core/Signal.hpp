#pragma once

namespace RR
{
    namespace Rfx
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
        #define RFX_UNEXPECTED(reason) \
            handleSignal(SignalType::Unexpected, reason)

        #define RFX_UNIMPLEMENTED_X(what) \
            handleSignal(SignalType::Unimplemented, what)

        #define RFX_UNREACHABLE(msg) \
            handleSignal(SignalType::Unreachable, msg)

        //TODO ?
        #define RFX_ASSERT_FAILURE(msg) \
            handleSignal(SignalType::AssertFailure, msg)

        #define RFX_INVALID_OPERATION(msg) \
            handleSignal(SignalType::InvalidOperation, msg)

        #define RFX_ABORT_COMPILATION(msg) \
            handleSignal(SignalType::AbortCompilation, msg)
        // clang-format on
    }
}