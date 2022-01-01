#pragma once

#include "core/SourceLocation.hpp"

namespace RR
{
    namespace Rfx
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
            Token(TokenType inType, const UnownedStringSlice& stringSlice, const SourceLocation& SourceLocation, const HumaneSourceLocation& humaneSourceLocation)
                : type(inType), stringSlice(stringSlice), sourceLocation(SourceLocation), humaneSourceLocation(humaneSourceLocation)
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
            HumaneSourceLocation humaneSourceLocation;
        };
    }
}

template <>
struct fmt::formatter<RR::Rfx::TokenType> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::TokenType tokenType, FormatContext& ctx)
    {
        return formatter<string_view>::format(RR::Rfx::TokenTypeToString(tokenType), ctx);
    }
};
