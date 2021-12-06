#include "PreprocessorContext.hpp"

#include "compiler/Lexer.hpp"
//#include "compiler/Tokenizer.hpp"

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

               
                Lexer lexer(source);

                for (;;)
                {
                    const auto& Token = lexer.GetNextToken();

                    Log::Format::Info("Token:{{Type:\"{0}\", Content:\"{1}\"}}", TokenTypeToString(Token.type), Token.GetContentString());

                    if (Token.type == TokenType::EndOfFile)
                        break;
                }
            }
        }
    }
}