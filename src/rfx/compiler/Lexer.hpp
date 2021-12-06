#pragma once

#include "compiler/InputString.hpp"
#include "compiler/Token.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace Common
    {
        class LinearAllocator;
    }

    namespace Rfx
    {
        namespace Compiler
        {

            class Lexer final
            {
            public:
                using const_iterator = U8String::const_iterator;
                using iterator = U8String::iterator;

                Lexer() = delete;
                Lexer(const U8String& source);
                ~Lexer();

                Token GetNextToken();

            private:
                enum class Flags : uint32_t
                {
                    None = 0 << 0,
                    AtStartOfLine = 1 << 0,
                    AfterWhitespace = 1 << 1,
                };
                ENUM_CLASS_FRIEND_OPERATORS(Flags)

            private:
                inline bool isReachEOF()
                {
                    return cursor_ == end_;
                }

                inline U8Char peek()
                {
                    ASSERT(!isReachEOF());
                    return *cursor_;
                }

                TokenType scanToken(uint32_t& escapedLines);

                bool advance(uint32_t& escapedLines);
                void handleWhiteSpace(uint32_t& escapedLines);
                void handleLineComment(uint32_t& escapedLines);
                void handleBlockComment(uint32_t& escapedLines);

                void lexNumberSuffix(uint32_t& escapedLines);
                void lexDigits(uint32_t& escapedLines, uint32_t base);
                TokenType lexNumber(uint32_t& escapedLines, uint32_t base);
                void lexNumberAfterDecimalPoint(uint32_t& escapedLines, uint32_t base);
                bool maybeLexNumberExponent(uint32_t& escapedLines, uint32_t base);

                void lexIdentifier(uint32_t& escapedLines);
                void lexStringLiteralBody(uint32_t& escapedLines, U8Char quote);

            private:
                Flags flags_;
                uint32_t line_ = 1;

                const_iterator cursor_;
                const_iterator end_;

                std::unique_ptr<LinearAllocator> allocator_;
            };
        }
    }
}