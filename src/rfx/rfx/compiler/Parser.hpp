#pragma once

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
        class LinearAllocator;
    }

    namespace Rfx
    {
        class ParserImpl;
        class DiagnosticSink;
        class ASTBuilder;
        class SourceView;
        struct CompileContext;
        struct Token;
        struct TokenSpan;
        struct JSONValue;

        class Parser final : Common::NonCopyable
        {
        public:
            ~Parser();
            Parser() = delete;
            Parser(const TokenSpan& tokenSpan, JSONValue& root, const std::shared_ptr<CompileContext>& context);

            Common::RResult Parse();

        private:
            std::unique_ptr<ParserImpl> impl_;
        };
    }
}