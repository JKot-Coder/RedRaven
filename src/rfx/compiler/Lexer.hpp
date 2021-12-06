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
                    Eof,
                    WhiteSpace,

                    Pound,
                    If,
                    IfDef,
                    IfNdef,
                    Else,
                    Elif,
                    EndIf,
                    Include,
                    Define,
                    Undef,
                    Warning,
                    Error,
                    Line,
                    Pragma,
                    UnknownDirective,

                    NewLine,
                };

                Token() = delete;

                Token(Type inType, const U8Char* inContent, size_t inPosition, size_t inLength)
                    : type(inType), content(inContent), position(inPosition), length(inLength)
                {
                }

                U8String GetContentString() const
                {
                    if (!content || !length)
                        return "";

                    return U8String(content, length);
                }

                const U8Char* content;
                size_t position;
                size_t length;

                Type type;
            };

            class Lexer final
            {
            public:
                Lexer() = delete;
                Lexer(const U8String& source);

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

                inline bool increment()
                {
                    ASSERT(!isReachEOF());

                    cursor_++;
                    return !isReachEOF();
                }

                Token::Type scanToken();

            private:
                Flags flags_;

                U8String::const_iterator cursor_;
                U8String::const_iterator end_;
            };

        }
    }
}