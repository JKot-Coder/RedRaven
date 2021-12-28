#pragma once

#include "compiler/Token.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            class SourceFile;

            struct InputStream;

            struct PathInfo;
            struct SourceLocation;

            class DiagnosticSink;
            struct DiagnosticInfo;

            class Lexer;

            class Preprocessor final
            {
            public:
                struct DirectiveContext;

            public:
                Preprocessor() = delete;
                Preprocessor(const std::shared_ptr<SourceFile>& sourceFile, const std::shared_ptr<DiagnosticSink>& diagnosticSink);
                ~Preprocessor();

                // read the entire input into tokens
                std::vector<Token> ReadAllTokens();
                Token ReadToken();

            private:
                int32_t tokenToInt(const Token& token, int radix);
                uint32_t tokenToUInt(const Token& str, int radix);

                // TODO comments
                /// Push a new input source onto the input stack of the preprocessor
                void pushInputStream(const std::shared_ptr<InputStream>& inputStream);

                /// Pop the inner-most input source from the stack of input source
                void popInputStream();

                Token peekRawToken();
                TokenType peekRawTokenType() { return peekRawToken().type; }

                Token peekToken();
                TokenType peekTokenType() { return peekToken().type; }

                // Read one token, with macro-expansion, without going past the end of the line.
                Token advanceToken();

                // Read one raw token, without going past the end of the line.
                Token advanceRawToken();

                // Skip to the end of the line (useful for recovering from errors in a directive)
                void skipToEndOfLine();

                bool expectRaw(DirectiveContext& context, TokenType expected, DiagnosticInfo const& diagnostic);

                // Determine if we have read everything on the directive's line.
                bool isEndOfLine();

                void handleDirective(DirectiveContext& directiveContext);
                void handleDefineDirective(DirectiveContext& directiveContext);
                void handleLineDirective(DirectiveContext& directiveContext);

                // Helper routine to check that we find the end of a directive where
                // we expect it.
                //
                // Most directives do not need to call this directly, since we have
                // a catch-all case in the main `handleDirective()` function.
                // `#include` and `#line` case will call it directly to avoid complications
                // when it switches the input stream.
                void expectEndOfDirective(DirectiveContext& context);

            private:
                std::shared_ptr<InputStream> currentInputStream_;
                std::shared_ptr<SourceFile> sourceFile_;
                std::shared_ptr<DiagnosticSink> sink_;
                // std::unique_ptr<Lexer> lexer_;

                /// A pre-allocated token that can be returned to represent end-of-input situations.
                Token endOfFileToken_;
            };
        }
    }
}