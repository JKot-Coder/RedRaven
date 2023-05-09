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
        struct Token;

        class Parser
        {
        public:
            ~Parser();
            Parser() = delete;
            Parser(const std::shared_ptr<SourceView>& sourceView,
                   const std::shared_ptr<Common::LinearAllocator>& allocator,
                   const std::shared_ptr<DiagnosticSink>& diagnosticSink);

            Common::RResult Parse(const std::shared_ptr<ASTBuilder>& astBuilder);

        private:
            // TODO tempoprary shared. Is it possible not to use it?
            std::unique_ptr<ParserImpl> impl_;
        };
    }
}