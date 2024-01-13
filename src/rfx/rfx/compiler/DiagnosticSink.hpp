#pragma once

#include "rfx/core/SourceLocation.hpp"
#include "rfx/core/UnownedStringSlice.hpp"

namespace RR
{
    namespace Rfx
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
            const char* name; ///< Unique name
            const char* messageFormat;
        };

        struct Diagnostic
        {
            Diagnostic() = default;

            U8String message;
            SourceLocation location;
            int32_t errorID = -1;
            Severity severity;
            UnownedStringSlice stringSlice;
            bool isTokenValid = false;
        };

        class DiagnosticSink final
        {
        public:
            class IWriter
            {
            public:
                virtual void Write(const U8String& message) = 0;
            };

            DiagnosticSink(bool onlyRelativePaths) : onlyRelativePaths_(onlyRelativePaths) {};

            template <typename... Args>
            inline void Diagnose(const SourceLocation& location, const DiagnosticInfo& info, Args&&... args)
            {
                Diagnostic diagnostic;
                diagnostic.errorID = info.id;
                diagnostic.message = fmt::format(info.messageFormat, args...);
                diagnostic.location = location;
                diagnostic.severity = info.severity;

                diagnoseImpl(info, formatDiagnostic(diagnostic));
            }

            template <typename T, typename... Args>
            inline void Diagnose(const T& token, const DiagnosticInfo& info, Args&&... args)
            {
                Diagnostic diagnostic;
                diagnostic.errorID = info.id;
                diagnostic.message = fmt::format(info.messageFormat, args...);
                diagnostic.location = token.sourceLocation;
                diagnostic.isTokenValid = token.isValid();
                diagnostic.stringSlice = token.stringSlice;
                diagnostic.severity = info.severity;

                diagnoseImpl(info, formatDiagnostic(diagnostic));
            }

            template <typename... Args>
            inline void Diagnose(DiagnosticInfo const& info, Args const&... args)
            {
                ASSERT(info.severity == Severity::Note || info.id == 12345) // Ignore cutom error.

                Diagnostic diagnostic;
                diagnostic.errorID = info.id;
                diagnostic.message = fmt::format(info.messageFormat, args...);
                diagnostic.location = {};
                diagnostic.isTokenValid = false;
                diagnostic.stringSlice = {};
                diagnostic.severity = info.severity;

                diagnoseImpl(info, formatDiagnostic(diagnostic));
            }

            void AddWriter(const std::shared_ptr<IWriter>& writer)
            {
                ASSERT(writer);
                writerList_.push_back(writer);
            }

            /// Set the maximum length (in chars) of a source line displayed. Set to 0 for no limit
            void SetSourceLineMaxLength(size_t length) { sourceLineMaxLength_ = length; }
            size_t GetSourceLineMaxLength() const { return sourceLineMaxLength_; }

            /// Get the total amount of errors that have taken place on this DiagnosticSink
            inline uint32_t GetErrorCount() { return errorCount_; }

        private:
            template <typename T, typename... Args>
            inline U8String formatDiagnostic(const T& token, const DiagnosticInfo& info, Args&&... args)
            {
                Diagnostic diagnostic;
                diagnostic.errorID = info.id;
                diagnostic.message = fmt::format(info.messageFormat, args...);
                diagnostic.location = token.sourceLocation;
                diagnostic.isTokenValid = token.isValid();
                diagnostic.stringSlice = token.stringSlice;
                diagnostic.severity = info.severity;

                return formatDiagnostic(diagnostic);
            }

            U8String formatDiagnostic(const Diagnostic& diagnostic);

            void diagnoseImpl(const DiagnosticInfo& info, const U8String& formattedMessage);

        private:
            uint32_t errorCount_ = 0;
            size_t sourceLineMaxLength_ = 120;
            std::vector<std::shared_ptr<IWriter>> writerList_;
            bool onlyRelativePaths_ = false;
        };

        class BufferWriter final : public DiagnosticSink::IWriter
        {
        public:
            void Write(const U8String& message) override { buffer_ += message; }
            U8String GetBuffer() const { return buffer_; }

        private:
            U8String buffer_;
        };
    }
}