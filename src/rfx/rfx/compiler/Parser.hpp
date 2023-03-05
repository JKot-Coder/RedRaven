#pragma once

namespace RR::Rfx
{
    class ParserImpl;
    class DiagnosticSink;
    class ASTBuilder;
    struct Token;

    class Parser
    {
    public:
        ~Parser();
        Parser() = delete;
        Parser(const std::shared_ptr<DiagnosticSink>& diagnosticSink);

        void Parse(const std::vector<Token>& tokens,
                   const std::shared_ptr<ASTBuilder>& astBuilder);

    private:
        // TODO tempoprary shared. Is it possible not to use it?
        std::unique_ptr<ParserImpl> impl_;
    };
}