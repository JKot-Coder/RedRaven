#pragma once

#include "rfx/core/SourceLocation.hpp"
#include "rfx/core/UnownedStringSlice.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR::Rfx
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
            EscapedNewLines = 1 << 2,
            EscapedCharacters = 1 << 3,
        };
        ENUM_CLASS_FRIEND_BITWISE_OPS(Flags)

    public:
        Token() = default;
        Token(Type inType, const UnownedStringSlice& stringSlice, const SourceLocation& sourceLocation, Flags flags = Flags::None)
            : type(inType), flags(flags), stringSlice(stringSlice), sourceLocation(sourceLocation)
        {
        }

        U8String GetContentString() const
        {
            if (stringSlice.length() == 0)
                return "";

            return U8String(stringSlice.begin(), stringSlice.end());
        }

        inline bool isValid() const { return type != Type::Unknown; }

    public:
        Type type = Type::Unknown;
        Flags flags = Flags::None;
        UnownedStringSlice stringSlice;
        SourceLocation sourceLocation;
    };

    template <typename T, typename = void>
    struct IsTokenStream : std::false_type
    {
    };

    template <typename T>
    struct IsTokenStream<T, std::void_t<decltype(std::declval<T>().AdvanceToken()),
                                        decltype(std::declval<T>().PeekToken()),
                                        decltype(std::declval<T>().PeekTokenType())>> : std::true_type
    {
    };

    U8String TokenTypeToString(Token::Type type);

    using TokenList = std::vector<Token>;

    struct TokenSpan
    {
    public:
        using iterator = TokenList::iterator;
        using const_iterator = TokenList::const_iterator;

    public:
        TokenSpan() = default;
        TokenSpan(const TokenList& tokenList)
            : begin_(tokenList.begin()), end_(tokenList.end()) { }

        const_iterator begin() const { return begin_; }
        const_iterator end() const { return end_; }
        size_t size() { return std::distance(end_, begin_); }

    private:
        const_iterator begin_;
        const_iterator end_;
    };

    struct TokenReader
    {
    public:
        using const_iterator = TokenList::const_iterator;

        TokenReader() = default;

        explicit TokenReader(const TokenSpan& tokens)
            : cursor_(tokens.begin()), end_(tokens.end())
        {
            updateLookaheadToken();
        }

        explicit TokenReader(const TokenList& tokens)
            : cursor_(tokens.begin()), end_(std::prev(tokens.end()))
        {
            updateLookaheadToken();
        }

        explicit TokenReader(const_iterator begin, const_iterator end)
            : cursor_(begin), end_(end)
        {
            updateLookaheadToken();
        }

        struct ParsingCursor
        {
            Token nextToken;
            const_iterator tokenReaderCursor;
        };

        ParsingCursor GetCursor()
        {
            ParsingCursor rs;
            rs.nextToken = nextToken_;
            rs.tokenReaderCursor = cursor_;
            return rs;
        }

        void SetCursor(ParsingCursor cursor)
        {
            cursor_ = cursor.tokenReaderCursor;
            nextToken_ = cursor.nextToken;
        }

        bool IsAtCursor(const ParsingCursor& cursor) const
        {
            return cursor.tokenReaderCursor == cursor_;
        }

        Token AdvanceToken();

        bool IsAtEnd() const { return cursor_ == end_; }
        const Token& PeekToken() const { return nextToken_; }
        Token::Type PeekTokenType() const { return nextToken_.type; }
        /// Peek the location of the next token in the input stream.
        SourceLocation PeekLoc() { return PeekToken().sourceLocation; }

    private:
        /// Update the lookahead token in `nextToken_` to reflect the cursor state
        void updateLookaheadToken();

    private:
        Token nextToken_;
        TokenList::const_iterator cursor_;
        TokenList::const_iterator end_;
    };
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