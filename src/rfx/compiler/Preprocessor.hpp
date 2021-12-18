#pragma once

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            class SourceFile;
            struct SourceLocation;
            class DiagnosticSink;
            struct DiagnosticInfo;
            class Lexer;
            struct Token;
            enum class TokenType : uint32_t;

            class Preprocessor final
            {
            public:
                struct DirectiveContext;

            public:
                Preprocessor() = delete;
                Preprocessor(const std::shared_ptr<SourceFile>& sourceFile, const std::shared_ptr<DiagnosticSink>& diagnosticSink);
                ~Preprocessor();

                // read the entire input into tokens
                std::shared_ptr<std::vector<Token>> ReadAllTokens();
                Token ReadToken();

            private:
                struct InputSource;

            private:
                /// Push a new input source onto the input stack of the preprocessor
                void pushInputSource(const std::shared_ptr<InputSource>& inputSource);

                /// Pop the inner-most input source from the stack of input source
                void popInputSource();

                /// Peek the location of the next token in the input stream.
                SourceLocation peekLoc();
                /// Peek the type of the next token in the input stream.
                TokenType peekRawTokenType();

                bool expectRaw(DirectiveContext& context, TokenType expected, DiagnosticInfo const& diagnostic);

                void handleDirective(DirectiveContext& directiveContext);

                void handleDefineDirective(DirectiveContext& directiveContext);

            private:
                std::shared_ptr<InputSource> currentInputSource_;
                std::shared_ptr<SourceFile> sourceFile_;
                std::shared_ptr<DiagnosticSink> sink_;
                std::unique_ptr<Lexer> lexer_;
            };
        }
    }
}