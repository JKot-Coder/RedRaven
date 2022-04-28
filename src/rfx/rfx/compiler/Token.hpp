#pragma once

#include "core/SourceLocation.hpp"

#include "common/EnumClassOperators.hpp"

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
            enum class Flags : uint32_t
            {
                None = 0 << 0,
                AtStartOfLine = 1 << 0,
                AfterWhitespace = 1 << 1,
                EscapedNewLines = 1 << 2
            };
            ENUM_CLASS_FRIEND_OPERATORS(Flags)

        public:
            Token() = default;
            Token(Token::Type inType, const UnownedStringSlice& stringSlice, const SourceLocation& SourceLocation, Flags flags = Flags::None)
                : type(inType), flags(flags), stringSlice(stringSlice), sourceLocation(SourceLocation)
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
            Flags flags = Flags::None;
            UnownedStringSlice stringSlice;
            SourceLocation sourceLocation;
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