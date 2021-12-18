#pragma once

#include "compiler/UnownedStringSlice.hpp"
#include "compiler/SourceLocation.hpp"

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

            struct Token 
            {
                Token() = default;
                Token(TokenType inType, const UnownedStringSlice& stringSlice, const SourceLocation& SourceLocation)
                    : type(inType), stringSlice(stringSlice), sourceLocation(SourceLocation)
                {
                }

                U8String GetContentString() const
                {
                    if (stringSlice.GetLength() == 0)
                        return "";

                    return U8String(stringSlice.Begin(), stringSlice.End());
                }

                inline bool isValid() const { return type != TokenType::Unknown; }

                TokenType type = TokenType::Unknown;
                UnownedStringSlice stringSlice;
                SourceLocation sourceLocation;
            };
        }
    }
}