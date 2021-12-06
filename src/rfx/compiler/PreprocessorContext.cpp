#include "PreprocessorContext.hpp"

#include "compiler/Lexer.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            PreprocessorContext::PreprocessorContext()
            {
            }

            U8String getTokenName(Token::Type type)
            {
                switch (type)
                {
                    case Token::Type::Eof:
                        return "Eof";
                    case Token::Type::NewLine:
                        return "NewLine";
                    case Token::Type::WhiteSpace:
                        return "WhiteSpace";
                    case Token::Type::Include:
                        return "Include";
                    default:
                        return "???";
                }
            }

            void PreprocessorContext::Parse(const U8String& source)
            {
                Lexer lexer(source);

                for (;;)
                {
                    const auto& token = lexer.GetNextToken();

                    Log::Format::Info("Token:{{Type:\"{0}\", Content:\"{1}\"}}", getTokenName(token.type), token.GetContentString());

                    if (token.type == Token::Type::Eof)
                        break;
                }
            }
        }
    }
}