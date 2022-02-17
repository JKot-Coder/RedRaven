#pragma once

#include "core/SourceLocation.hpp"

namespace RR
{
    namespace Rfx
    {
        struct Token
        {
        public:
            enum class Type : uint32_t
            {
#define TOKEN(NAME, DESC) NAME,
#include "TokenDefinitions.hpp"
            };

        public:
            Token() = default;
            Token(Token::Type inType, const UnownedStringSlice& stringSlice, const SourceLocation& SourceLocation, const HumaneSourceLocation& humaneSourceLocation)
                : type(inType), stringSlice(stringSlice), sourceLocation(SourceLocation), humaneSourceLocation(humaneSourceLocation)
            {
            }

            U8String GetContentString() const
            {
                if (stringSlice.GetLength() == 0)
                    return "";

                return U8String(stringSlice.Begin(), stringSlice.End());
            }

            inline bool isValid() const { return type != Token::Type::Unknown; }

        public:
            Token::Type type = Token::Type::Unknown;
            UnownedStringSlice stringSlice;
            SourceLocation sourceLocation;
            HumaneSourceLocation humaneSourceLocation;
        };

        U8String TokenTypeToString(Token::Type type);
    }
}

template <>
struct fmt::formatter<RR::Rfx::Token::Type> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::Token::Type tokenType, FormatContext& ctx)
    {
        return formatter<string_view>::format(RR::Rfx::TokenTypeToString(tokenType), ctx);
    }
};