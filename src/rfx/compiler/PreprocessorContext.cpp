#include "PreprocessorContext.hpp"

#include "compiler/Lexer.hpp"
#include "compiler/DiagnosticSink.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            PreprocessorContext::PreprocessorContext()
            {
            }

            void PreprocessorContext::Parse(const U8String& source)
            {
               /* Tokenizer tokenizer(source);

                for (;;)
                {
                    const auto& token = tokenizer.GetNextToken();

                    Log::Format::Info("Token:{{Type:\"{0}\", Content:\"{1}\"}}", getTokenName(token.type), token.GetContentString());

                    if (token.type == TokenType::Eof)
                        break;
                }*/
                SourceFile* sourceFile = new SourceFile();//, SourceRange range, const U8String* viewPath, SourceLocation initiatingSourceLocation)

                sourceFile->SetContents(source);

                auto diagnosticSink = std::make_shared<DiagnosticSink>();
                auto sourceView = std::make_shared<SourceView>(sourceFile, nullptr);

                Lexer lexer(sourceView, diagnosticSink);

                for (;;)
                {
                    const auto& Token = lexer.GetNextToken();

                    Log::Format::Info("Token:{{Type:\"{0}\", Content:\"{1}\", Line:{2} Column:{3}}}", TokenTypeToString(Token.type), Token.GetContentString(), Token.SourceLocation.line, Token.SourceLocation.column);

                    if (Token.type == TokenType::EndOfFile)
                        break;
                }
            }
        }
    }
}