#pragma once

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            enum class TokenType : uint32_t
            {
#define TOKEN(NAME, DESC) NAME,
#include "TokenDefinitions.hpp"
            };

            U8String TokenTypeToString(TokenType type);

            struct Token : NonCopyable
            {

                Token() = delete;

                Token(TokenType inType, const U8Char* inContent, size_t inLength, uint32_t inLine)
                    : type(inType), content(inContent), length(inLength), line(inLine)
                {
                }

                U8String GetContentString() const
                {
                    if (!content || !length)
                        return "";

                    return U8String(content, length);
                }

                const U8Char* content;
                size_t length;
                uint32_t line;

                TokenType type;
            };
        }
    }
}