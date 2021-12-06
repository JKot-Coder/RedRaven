#include "Lexer.hpp"

#include "compiler/Tokenizer.hpp"

#include <iterator>

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            namespace
            {
                inline bool isWhiteSpace(U8Char ch)
                {
                    return (ch == ' ' || ch == '\t');
                }
                /*
                Token::Type getTokenForPreprocessorDirective(U8String directive)
                {
                    static const std::unordered_map<U8String, Token::Type> directives = {
                        { "if", Token::Type::If },
                        { "ifdef", Token::Type::IfDef },
                        { "ifndef", Token::Type::IfNdef },
                        { "else", Token::Type::Else },
                        { "elif", Token::Type::Elif },
                        { "endif", Token::Type::EndIf },
                        { "include", Token::Type::Include },
                        { "define", Token::Type::Define },
                        { "undef", Token::Type::Undef },
                        { "warning", Token::Type::Warning },
                        { "error", Token::Type::Error },
                        { "line", Token::Type::Line },
                        { "pragma", Token::Type::Pragma },
                        { "", Token::Type::Pound }
                    };

                    auto search = directives.find(directive);
                    if (search == directives.end())
                        return Token::Type::UnknownDirective;

                    return search->second;
                }*/
            }

            Lexer::Lexer(const U8String& source)
                : tokenizer_(new Tokenizer(source))
            {
            }

            Lexer::~Lexer()
            {
            }

            Lexeme Lexer::GetNextLexeme()
            {
                const auto tokenBegin = cursor_;

                const auto tokenType = scanToken();

                const auto tokenEnd = cursor_;
                #ifdef QWEQWE
                    qweqwe
                #endif

                switch (tokenType)
                {
                    case Token::Type::NewLine:
                    {
                        // If we just reached the end of a line, then the next token
                        // should count as being at the start of a line, and also after
                        // whitespace.
                        //flags = Flags::AtStartOfLine | Flags::AfterWhitespace;
                        break;
                    }
                    case Token::Type::WhiteSpace:
                        // case TokenType::BlockComment:
                        // case TokenType::LineComment:
                        {
                            // True horizontal whitespace and comments both count as whitespace.
                            //
                            // Note that a line comment does not include the terminating newline,
                            // we do not need to set `AtStartOfLine` here.
                            flags_ |= Flags::AfterWhitespace;
                            break;
                        }

                    default:
                    {
                        // If we read some token other then the above cases, then we are
                        // neither after whitespace nor at the start of a line.
                        flags_ = Flags::None;
                        break;
                    }
                }*/

                return Lexeme(Lexeme::Type::Define, nullptr, 0, 0);
                //  return Token(tokenType, &*tokenBegin, 0, uint32_t(std::distance(tokenBegin, tokenEnd)));
            }

            Lexeme::Type Lexer::scanLexeme()
            {
                if (cursor_ == end_)
                {
                    const auto& token = tokenizer_->GetNextToken();

                    cursor_ = token.begin;
                    end_ = token.begin + token.lenght;

                    switch (token.type)
                    {
                        case Token::Type::Eof:
                            return Lexeme::Type::Eof;
                        case Token::Type::NewLine:
                            return Lexeme::Type::NewLine;
                    }
                }

                ASSERT(cursor_ != end_)

                switch (peek())
                {
                    /* case '\r':
                    case '\n':
                    {
                        if (!increment())
                            return Token::Type::NewLine;

                        // Handle all newline sequences
                        //  "\n"
                        //  "\r"
                        //  "\r\n"
                        //  "\n\r"
                        const auto next = peek();
                        if ((next == '\r' || next == '\n') && next != ch)
                            increment();

                        return Token::Type::NewLine;
                    }
                    case '#':
                    {
                        if (!increment())
                            return Token::Type::Pound;

                        // Preprocessor directives always on start the line or after whitspace
                        if ((flags_ & Flags::AtStartOfLine) != Flags::None ||
                            (flags_ & Flags::AfterWhitespace) != Flags::None)
                        {
                            const auto begin = cursor_;

                            while (!isWhiteSpace(peek())) // Not line ending // Not
                            {
                                if (!increment())
                                    break;
                            }

                            const auto end = cursor_;

                            return getTokenForPreprocessorDirective(U8String(begin, end));
                        }
                    }
                    */
                    case '!':
                    default:
                    {
                        for (;;)
                        {
                            if (!advance())
                                break;
                        }

                        return Lexeme::Type::WhiteSpace;
                    }
                }
            }
        }
    }
}