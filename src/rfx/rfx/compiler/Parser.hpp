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

        class Parser
        {
        public:
            ~Parser();
            Parser() = delete;
            Parser(const std::shared_ptr<SourceView>& sourceView,
                   const std::shared_ptr<CompileContext>& context);

            Common::RResult Parse();

        private:
            // TODO tempoprary shared. Is it possible not to use it?
            std::unique_ptr<ParserImpl> impl_;
        };
    }
}