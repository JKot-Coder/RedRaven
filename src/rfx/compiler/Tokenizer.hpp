#pragma once

#include "compiler/InputString.hpp"

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

            struct Token : NonCopyable
            {
                enum class Type : uint32_t
                {
                    Lexeme,
                    NewLine,
                    Eof,
                };

                Token() = delete;

                Token(Type inType, const U8Char* inBegin, size_t inLenght, uint32_t inLine)
                    : type(inType), begin(inBegin), lenght(inLenght), line(inLine)
                {
                }

                U8String GetContentString() const
                {
                    if (!begin || !lenght)
                        return "";

                    return U8String(begin, lenght);
                }

                Type type;
                const U8Char* begin;
                size_t lenght;
                uint32_t line;
            };

            class Tokenizer final
            {
            public:
                using const_iterator = U8String::const_iterator;
                using iterator = U8String::iterator;

                Tokenizer() = delete;
                Tokenizer(const U8String& source);

                ~Tokenizer();

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
                std::unique_ptr<LinearAllocator> allocator_;
            };

        }
    }
}