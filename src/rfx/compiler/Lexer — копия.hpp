#pragma once

#include "compiler/InputString.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            class Tokenizer;

            struct Lexeme : NonCopyable
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

                Lexeme() = delete;

                Lexeme(Type inType, const U8Char* inContent, size_t inLength, uint32_t inLine)
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

                Type type;
            };

            class Lexer final
            {
            public:
                Lexer() = delete;
                Lexer(const U8String& source);
                ~Lexer();

                Lexeme GetNextLexeme();

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

                inline bool advance()
                {
                    ASSERT(!isReachEOF());

                    cursor_++;
                    return !isReachEOF();
                }

                Lexeme::Type scanLexeme();

            private:
                Flags flags_;
    
                std::unique_ptr<Tokenizer> tokenizer_;
                const U8Char* cursor_ = nullptr;
                const U8Char* end_ = nullptr;
            };

        }
    }
}