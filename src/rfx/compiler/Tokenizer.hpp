#pragma once

#include "compiler/InputString.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {

            struct Token
            {
                enum class Type : uint32_t
                {
                    Lex,
                    WhiteSpace,
                    NewLine,
                    Eof,
                };

                Token() = delete;

                Token(Type inType, const U8String::const_iterator inBegin, const U8String::const_iterator inEnd, uint32_t inLine)
                    : type(inType), begin(inBegin), end(inEnd), line(inLine)
                {
                }

                Type type;
                const U8String::const_iterator begin;
                const U8String::const_iterator end;
                uint32_t line;               
            };

            class Tokenizer final
            {
            public:
                using const_iterator = U8String::const_iterator;
                using iterator = U8String::iterator;

                Tokenizer() = delete;
                Tokenizer(const U8String& source);

                Token GetNextToken();

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

                inline bool advance()
                {
                    ASSERT(!isReachEOF());

                    cursor_++;
                    return !isReachEOF();
                }

                Token::Type scanToken(bool& scrubbingNeeded);

            private:
                uint32_t line_ = 1;
                const_iterator cursor_;
                const_iterator end_;
            };

        }
    }
}