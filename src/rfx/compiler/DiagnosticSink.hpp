#pragma once

#include "compiler/SourceLocation.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            enum class Severity
            {
                Note,
                Warning,
                Error,
                Fatal,
                Internal,
            };

            U8String GetSeverityName(Severity severity);

            // A structure to be used in static data describing different
            // diagnostic messages.
            struct DiagnosticInfo
            {
                int32_t id;
                Severity severity;
                U8String name; ///< Unique name
                U8String messageFormat;
            };

            struct Diagnostic
            {
                Diagnostic() = default;
                Diagnostic(
                    const U8String& inMessage,
                    int id,
                    // const SourceLocation& pos,
                    Severity inSeverity)
                    : errorID(id),
                      message(inMessage),
                      severity(inSeverity)
                {
                }

                U8String message;
                SourceLocation location;
                int32_t errorID = -1;
                Severity severity;
            };

            class DiagnosticSink final
            {
            public:
                DiagnosticSink() = default;

                template <typename... Args>
                inline void Dispatch(SourceLocation const& location, const DiagnosticInfo& info, Args&&... args)
                {
                    Diagnostic diagnostic;
                    diagnostic.errorID = info.id;
                    diagnostic.message = fmt::format(info.messageFormat, args...);
                    diagnostic.location = location;
                    diagnostic.severity = info.severity;

                    diagnoseImpl(info, formatDiagnostic(diagnostic));
                }

                /// Get the total amount of errors that have taken place on this DiagnosticSink
                inline uint32_t GetErrorCount() { return errorCount_; }

            private:
                U8String formatDiagnostic(const Diagnostic& diagnostic);

                void diagnoseImpl(const DiagnosticInfo& info, const U8String& formattedMessage);

            private:
                uint32_t errorCount_ = 0;
            };
        }
    }
}