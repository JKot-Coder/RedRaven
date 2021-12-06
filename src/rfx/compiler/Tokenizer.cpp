#include "Tokenizer.hpp"

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

                inline bool isNewLineChar(U8Char ch)
                {
                    return (ch == '\n' || ch == '\r');
                }

                bool checkForEscapedNewline(const Tokenizer::const_iterator cursor, const Tokenizer::const_iterator end)
                {
                    ASSERT(*cursor == '\\')

                    U8Char next = 0;

                    // Peak next char if exist
                    if (std::distance(cursor, end) > 1)
                        next = *(cursor + 1);

                    return isNewLineChar(next);
                }

                void handleNewlineSequence(Tokenizer::const_iterator& cursor, const Tokenizer::const_iterator end)
                {
                    ASSERT(isNewLineChar(*cursor))

                    const auto first = *cursor;

                    if (++cursor == end)
                        return;

                    const auto second = *cursor;

                    // Handle all newline sequences
                    //  "\n"
                    //  "\r"
                    //  "\r\n"
                    //  "\n\r"
                    if (isNewLineChar(second) && first != second)
                        cursor++;
                }

                void handleEscapedNewline(Tokenizer::const_iterator& cursor, const Tokenizer::const_iterator end)
                {
                    ASSERT(checkForEscapedNewline(cursor, end))

                    cursor++;
                    handleNewlineSequence(cursor, end);
                }

                uint32_t scrubbingToken(const Tokenizer::const_iterator srcBegin, const Tokenizer::const_iterator srcEnd,
                                    const Tokenizer::iterator dstBegin, Tokenizer::const_iterator& dstEnd)
                {
                    uint32_t escapedLines = 0;
                    auto cursor = srcBegin;
                    auto dst = dstBegin;

                    while (cursor != srcEnd)
                    {
                        const auto ch = *cursor++;

                        if (ch == '/')
                        {
                            if (checkForEscapedNewline(cursor, srcEnd))
                            {
                                escapedLines++;
                                handleEscapedNewline(cursor, srcEnd);
                                continue;
                            }
                        }

                        *dst++ = ch;
                    }
                    dstEnd = dst;

                    return escapedLines;
                }

            }

            Tokenizer::Tokenizer(const U8String& source)
                : cursor_(source.begin()),
                  end_(source.end())
            {
            }

            Token Tokenizer::GetNextToken()
            {
                if (isReachEOF())
                    return Token(Token::Type::Eof, end_, end_, line_);

                const auto tokenBegin = cursor_;

                bool scrubbingNeeded;
                const auto tokenType = scanToken(scrubbingNeeded);

                const auto tokenEnd = cursor_;

                const auto tokenLine = line_;

                if (tokenType == Token::Type::NewLine)
                    line_++;

                if (scrubbingNeeded)
                {
                    // "scrubbing" token value here to remove escaped newlines...
                    // Only perform this work if we encountered an escaped newline while lexing this token
                    // Allocate space that will always be more than enough for stripped contents
                    const size_t allocationSize = std::distance(tokenBegin, tokenEnd);
                    const Tokenizer::iterator beginDst = (char*)m_memoryArena->allocateUnaligned(allocationSize);
                    Tokenizer::const_iterator endDst = beginDst + allocationSize;

                    // count scrubbing lines. Because of scrambling count of NewLineTokens != ñount of lines in file.
                    line_ += scrubbingToken(tokenBegin, tokenEnd, beginDst, endDst);

                    return Token(tokenType, beginDst, endDst, tokenLine);
                }

                return Token(tokenType, tokenBegin, tokenEnd, tokenLine);
            }

            Token::Type Tokenizer::scanToken(bool& scrubbingNeeded)
            {
                ASSERT(!isReachEOF())

                bool scrubbingNeeded = false;
                auto ch = peek();

                switch (ch)
                {
                    case '\r':
                    case '\n':
                    {
                        handleNewlineSequence(cursor_, end_);
                        return Token::Type::NewLine;
                    }
                    case ' ':
                    case '\t':
                    {
                        for (;;)
                        {
                            if (peek() == '/')
                            {
                                if (checkForEscapedNewline(cursor_, end_))
                                {
                                    scrubbingNeeded = true;
                                    handleEscapedNewline(cursor_, end_);
                                }
                            }

                            if (!isWhiteSpace(peek()))
                                break;

                            if (!advance())
                                break;
                        }

                        return Token::Type::WhiteSpace;
                    }
                    default:
                    {
                        for (;;)
                        {
                            if (peek() == '/')
                            {
                                if (checkForEscapedNewline(cursor_, end_))
                                {
                                    scrubbingNeeded = true;
                                    handleEscapedNewline(cursor_, end_);
                                }
                            }

                            if (isWhiteSpace(peek()) || isNewLineChar(peek()))
                                break;

                            if (!advance())
                                break;
                        }

                        return Token::Type::Lex;
                    }
                }
            }

        }
    }
}