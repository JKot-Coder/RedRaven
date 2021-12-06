#include "Lexer.hpp"

#include "common/LinearAllocator.hpp"
#include "common/OnScopeExit.hpp"

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

                bool isNumberExponent(U8Char ch, uint32_t base)
                {
                    switch (ch)
                    { // clang-format off
                        case 'e': case 'E': return (base == 10);
                        case 'p': case 'P': return (base == 16);
                        default: return false;
                    } // clang-format on
                }

                bool checkForEscapedNewline(const Lexer::const_iterator cursor, const Lexer::const_iterator end)
                {
                    ASSERT(*cursor == '\\')

                    U8Char next = 0;

                    // Peak next char if exist
                    if (std::distance(cursor, end) > 1)
                        next = *(cursor + 1);

                    return isNewLineChar(next);
                }

                void handleNewlineSequence(Lexer::const_iterator& cursor, const Lexer::const_iterator end)
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

                void handleEscapedNewline(Lexer::const_iterator& cursor, const Lexer::const_iterator end)
                {
                    ASSERT(checkForEscapedNewline(cursor, end))

                    cursor++;
                    handleNewlineSequence(cursor, end);
                }

                size_t scrubbingToken(const Lexer::const_iterator srcBegin, const Lexer::const_iterator srcEnd,
                                      U8Char* dstBegin)
                {
                    size_t lenght = 0;
                    auto cursor = srcBegin;
                    auto dst = dstBegin;

                    while (cursor != srcEnd)
                    {
                        if (*cursor == '\\')
                        {
                            if (checkForEscapedNewline(cursor, srcEnd))
                            {
                                handleEscapedNewline(cursor, srcEnd);
                                continue;
                            }
                        }

                        lenght++;
                        *dst++ = *cursor++;
                    }

                    return lenght;
                }

                /*
                TokenType getTokenForPreprocessorDirective(U8String directive)
                {
                    static const std::unordered_map<U8String, TokenType> directives = {
                        { "if", TokenType::If },
                        { "ifdef", TokenType::IfDef },
                        { "ifndef", TokenType::IfNdef },
                        { "else", TokenType::Else },
                        { "elif", TokenType::Elif },
                        { "endif", TokenType::EndIf },
                        { "include", TokenType::Include },
                        { "define", TokenType::Define },
                        { "undef", TokenType::Undef },
                        { "warning", TokenType::Warning },
                        { "error", TokenType::Error },
                        { "line", TokenType::Line },
                        { "pragma", TokenType::Pragma },
                        { "", TokenType::Pound }
                    };

                    auto search = directives.find(directive);
                    if (search == directives.end())
                        return TokenType::UnknownDirective;

                    return search->second;
                }*/
            }

            Lexer::Lexer(const U8String& source)
                : cursor_(source.begin()),
                  end_(source.end()),
                  allocator_(new LinearAllocator(1024))
            {
            }

            Lexer::~Lexer()
            {
            }

            Token Lexer::GetNextToken()
            {
                if (isReachEOF())
                    return Token(TokenType::EndOfFile, nullptr, 0, line_);

                const auto tokenBegin = cursor_;

                uint32_t escapedLines = 0;
                const auto tokenType = scanToken(escapedLines);

                const auto tokenEnd = cursor_;

                const auto tokenLine = line_;

                if (tokenType == TokenType::NewLine)
                    line_++;

                if (escapedLines > 0)
                {
                    // "scrubbing" token value here to remove escaped newlines...
                    // Only perform this work if we encountered an escaped newline while lexing this token
                    // Allocate space that will always be more than enough for stripped contents
                    const size_t allocationSize = std::distance(tokenBegin, tokenEnd);
                    const auto beginDst = (U8Char*)allocator_->Allocate(allocationSize);

                    const auto scrubbledTokenLenght = scrubbingToken(tokenBegin, tokenEnd, beginDst);

                    // count escaped lines. Because of scrambling count of NewLineTokens != ñount of lines in file.
                    line_ += escapedLines;

                    return Token(tokenType, beginDst, scrubbledTokenLenght, tokenLine);
                }

                /*
                switch (tokenType)
                {
                    case TokenType::NewLine:
                    {
                        // If we just reached the end of a line, then the next token
                        // should count as being at the start of a line, and also after
                        // whitespace.
                        //flags = Flags::AtStartOfLine | Flags::AfterWhitespace;
                        break;
                    }
                    case TokenType::WhiteSpace:
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

                return Token(tokenType, &*tokenBegin, uint32_t(std::distance(tokenBegin, tokenEnd)), line_);
            }

            TokenType Lexer::scanToken(uint32_t& escapedLines)
            {
                ASSERT(!isReachEOF())

                escapedLines = 0;

                switch (peek())
                {
                    case '\r':
                    case '\n':
                    {
                        handleNewlineSequence(cursor_, end_);
                        return TokenType::NewLine;
                    }
                    case ' ':
                    case '\t':
                    {
                        handleWhiteSpace(escapedLines);
                        return TokenType::WhiteSpace;
                    }
                    case '.':
                    {
                        if (!advance(escapedLines))
                            return TokenType::Dot;

                        switch (peek())
                        {
                            // clang-format off
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9': // clang-format on
                                lexNumberAfterDecimalPoint(escapedLines, 10);
                                return TokenType::FloatingPointLiteral;

                            case '.':
                                // Note: consuming the second `.` here means that
                                // we cannot back up and return a `.` token by itself
                                // any more. We thus end up having distinct tokens for
                                // `.`, `..`, and `...` even though the `..` case is
                                // not part of HLSL.
                                //
                                if (!advance(escapedLines))
                                    return TokenType::DotDot;

                                switch (peek())
                                {
                                    case '.':
                                        advance(escapedLines);
                                        return TokenType::Ellipsis;

                                    default:
                                        return TokenType::DotDot;
                                }

                            default:
                                return TokenType::Dot;
                        }
                    }

                    // clang-format off
                    case '1': case '2': case '3': case '4': case '5':
                    case '6': case '7': case '8': case '9': // clang-format on
                        return lexNumber(escapedLines, 10);
                    case '0':
                    {
                        // auto loc = _getSourceLoc(lexer);
                        if (!advance(escapedLines))
                            return TokenType::IntegerLiteral;

                        switch (peek())
                        {
                            default:
                                lexNumberSuffix(escapedLines);
                                return TokenType::IntegerLiteral;

                            case '.':
                                if (!advance(escapedLines))
                                    return TokenType::FloatingPointLiteral;

                                lexNumberAfterDecimalPoint(escapedLines, 10);
                                return TokenType::FloatingPointLiteral;

                            case 'x':
                            case 'X':
                                if (!advance(escapedLines))
                                {
                                    // TODO Diagnostic
                                    return TokenType::Unknown;
                                }

                                return lexNumber(escapedLines, 16);

                            case 'b':
                            case 'B':
                                if (!advance(escapedLines))
                                {
                                    // TODO Diagnostic
                                    return TokenType::Unknown;
                                }

                                return lexNumber(escapedLines, 2);

                            // clang-format off
                            case '0': case '1': case '2': case '3': case '4': 
                            case '5': case '6': case '7': case '8': case '9': // clang-format on
                                /*
                                // TODO DIAGNOSTIC 
                                
                                if (auto sink = lexer->getDiagnosticSink())
                                {
                                    sink->diagnose(loc, LexerDiagnostics::octalLiteral);
                                }*/
                                return lexNumber(escapedLines, 8);
                        }
                    }

                    // clang-format off
                    case 'a': case 'b': case 'c': case 'd': case 'e':
                    case 'f': case 'g': case 'h': case 'i': case 'j':
                    case 'k': case 'l': case 'm': case 'n': case 'o':
                    case 'p': case 'q': case 'r': case 's': case 't':
                    case 'u': case 'v': case 'w': case 'x': case 'y':
                    case 'z':
                    case 'A': case 'B': case 'C': case 'D': case 'E':
                    case 'F': case 'G': case 'H': case 'I': case 'J':
                    case 'K': case 'L': case 'M': case 'N': case 'O':
                    case 'P': case 'Q': case 'R': case 'S': case 'T':
                    case 'U': case 'V': case 'W': case 'X': case 'Y':
                    case 'Z': 
                    case '_': // clang-format on
                        lexIdentifier(escapedLines);
                        return TokenType::Identifier;

                    case '\"':
                        if (!advance(escapedLines))
                            // TODO diagnostic
                            return TokenType::StringLiteral;

                        lexStringLiteralBody(escapedLines, '\"');
                        return TokenType::StringLiteral;

                    case '\'':
                        if (!advance(escapedLines))
                            // TODO diagnostic
                            return TokenType::CharLiteral;

                        lexStringLiteralBody(escapedLines, '\'');
                        return TokenType::CharLiteral;

                    case '+':
                        if (!advance(escapedLines))
                            return TokenType::OpAdd;

                        switch (peek())
                        { // clang-format off
                            case '+': advance(escapedLines); return TokenType::OpInc;
                            case '=': advance(escapedLines); return TokenType::OpAddAssign;
                            default: return TokenType::OpAdd; 
                        } // clang-format on

                    case '-':
                        if (!advance(escapedLines))
                            return TokenType::OpAdd;

                        switch (peek())
                        { // clang-format off
                            case '-': advance(escapedLines); return TokenType::OpDec;
                            case '=': advance(escapedLines); return TokenType::OpSubAssign;
                            case '>': advance(escapedLines); return TokenType::RightArrow;
                            default: return TokenType::OpSub;
                        } // clang-format on

                    case '*':
                        if (!advance(escapedLines))
                            return TokenType::OpMul;

                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpMulAssign;
                            default: return TokenType::OpMul;
                        } // clang-format on

                    case '/':
                        if (!advance(escapedLines))
                            return TokenType::OpDiv;

                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpDivAssign;
                            case '/': handleLineComment(escapedLines); return TokenType::LineComment;
                            case '*': handleBlockComment(escapedLines); return TokenType::BlockComment;     
                            default: return TokenType::OpDiv;
                        } // clang-format on  

                    case '%':
                        if (!advance(escapedLines))
                            return TokenType::OpMod;

                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpModAssign;
                            default: return TokenType::OpMod;
                        } // clang-format on  
                    case '|':
                        if (!advance(escapedLines))
                            return TokenType::OpBitOr;

                        switch (peek())
                        { // clang-format off
                            case '|': advance(escapedLines); return TokenType::OpOr;
                            case '=': advance(escapedLines); return TokenType::OpOrAssign;
                            default: return TokenType::OpBitOr;
                        } // clang-format on  

                    case '&':
                        if (!advance(escapedLines))
                            return TokenType::OpBitAnd;

                        switch (peek())
                        { // clang-format off
                            case '&': advance(escapedLines); return TokenType::OpAnd;
                            case '=': advance(escapedLines); return TokenType::OpAndAssign;
                            default: return TokenType::OpBitAnd;
                        } // clang-format on  

                    case '^':
                        if (!advance(escapedLines))
                            return TokenType::OpBitXor;
                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpXorAssign;
                            default: return TokenType::OpBitXor;
                        } // clang-format on  

                    case '>':
                        if (!advance(escapedLines))
                            return TokenType::OpGreater;

                        switch (peek())
                        { // clang-format off
                            case '>':
                                if (!advance(escapedLines))
                                    return TokenType::OpRsh;

                                switch (peek())
                                {
                                    case '=': advance(escapedLines); return TokenType::OpShrAssign;
                                    default: return TokenType::OpRsh;
                                }

                            case '=': advance(escapedLines); return TokenType::OpGeq;
                            default: return TokenType::OpGreater;
                        } // clang-format on 

                    case '<':
                        if (!advance(escapedLines))
                            return TokenType::OpLess;

                        switch (peek())
                        { // clang-format off
                            case '<': 
                                if (!advance(escapedLines)) 
                                    return TokenType::OpLsh;

                                switch (peek())
                                {
                                    case '=': advance(escapedLines); return TokenType::OpShlAssign;
                                    default: return TokenType::OpLsh;
                                }
                            case '=': advance(escapedLines); return TokenType::OpLeq;
                            default: return TokenType::OpLess;
                        } // clang-format on 

                    case '=':
                        if (!advance(escapedLines))
                            return TokenType::OpAssign;

                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpEql;
                            default: return TokenType::OpAssign;
                        } // clang-format on 

                    case '!':
                        if (!advance(escapedLines))
                            return TokenType::OpNot;

                        switch (peek())
                        { // clang-format off
                            case '=': advance(escapedLines); return TokenType::OpNeq;
                            default: return TokenType::OpNot;
                        } // clang-format on 

                    case '#':
                        if (!advance(escapedLines))
                            return TokenType::Pound;

                        switch (peek())
                        { // clang-format off
                            case '#': advance(escapedLines); return TokenType::PoundPound;
                            default: return TokenType::Pound;
                        } // clang-format on 

                    case '~':
                        advance(escapedLines);
                        return TokenType::OpBitNot;

                    case ':':
                    {
                        if (!advance(escapedLines))
                            return TokenType::Colon;

                        if (peek() == ':')
                        {
                            advance(escapedLines);
                            return TokenType::Scope;
                        }
                        return TokenType::Colon;
                    }
                    // clang-format off
                    case ';': advance(escapedLines); return TokenType::Semicolon;
                    case ',': advance(escapedLines); return TokenType::Comma;

                    case '{': advance(escapedLines); return TokenType::LBrace;
                    case '}': advance(escapedLines); return TokenType::RBrace;
                    case '[': advance(escapedLines); return TokenType::LBracket;
                    case ']': advance(escapedLines); return TokenType::RBracket;
                    case '(': advance(escapedLines); return TokenType::LParent;
                    case ')': advance(escapedLines); return TokenType::RParent;

                    case '?': advance(escapedLines); return TokenType::QuestionMark;
                    case '@': advance(escapedLines); return TokenType::At;
                    case '$': advance(escapedLines); return TokenType::Dollar;
                    // clang-format on

                    /*
                    case '#':
                    {
                        if (!increment())
                            return TokenType::Pound;

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
                    default:
                    {
                        for (;;)
                        {
                            if (!advance(escapedLines))
                                break;

                            if (isNewLineChar(peek()) || isWhiteSpace(peek()))
                                break;
                        }

                        return TokenType::Unknown;
                    }
                }
            }

            void Lexer::handleWhiteSpace(uint32_t& escapedLines)
            {
                ASSERT(isWhiteSpace(peek()))

                for (;;)
                {
                    if (!advance(escapedLines))
                        break;

                    if (!isWhiteSpace(peek()))
                        break;
                }
            }

            void Lexer::handleLineComment(uint32_t& escapedLines)
            {
                ASSERT(peek() == '/')

                for (;;)
                {
                    if (!advance(escapedLines))
                        break;

                    if (isNewLineChar(peek()))
                        break;
                }
            }

            void Lexer::handleBlockComment(uint32_t& escapedLines)
            {
                ASSERT(peek() == '*')

                for (;;)
                {
                    if (!advance(escapedLines))
                        // TODO diagnostic
                        return;

                    if (peek() == '*')
                    {
                        if (!advance(escapedLines))
                            // TODO diagnostic
                            return;

                        switch (peek())
                        {
                            case '/':
                                advance(escapedLines);
                                return;

                            default:
                                continue;
                        }
                    }
                }
            }

            void Lexer::lexNumberSuffix(uint32_t& escapedLines)
            {
                // Be liberal in what we accept here, so that figuring out
                // the semantics of a numeric suffix is left up to the parser
                // and semantic checking logic.
                for (;;)
                {
                    U8Char ch = peek();

                    // Accept any alphanumeric character, plus underscores.
                    if (('a' <= ch) && (ch <= 'z') || ('A' <= ch) && (ch <= 'Z') || ('0' <= ch) && (ch <= '9') || (ch == '_'))
                    {
                        if (!advance(escapedLines))
                            return;

                        continue;
                    }

                    // Stop at the first character that isn't
                    // alphanumeric.
                    return;
                }
            }

            void Lexer::lexDigits(uint32_t& escapedLines, uint32_t base)
            {
                for (;;)
                {
                    U8Char ch = peek();

                    int32_t digitVal = 0;
                    switch (ch)
                    { // clang-format off
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                            digitVal = ch - '0';
                            break;

                        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                            if (base <= 10) return;
                            digitVal = 10 + ch - 'a';
                            break;

                        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                            if (base <= 10) return;
                            digitVal = 10 + ch - 'A';
                            break;

                        default:
                            // Not more digits!
                            return;
                    } // clang-format on

                    if (digitVal >= int32_t(base))
                    { /*
                        if (auto sink = lexer->getDiagnosticSink())
                        {
                            char buffer[] = { (char)c, 0 };
                            sink->diagnose(_getSourceLoc(lexer), LexerDiagnostics::invalidDigitForBase, buffer, base);
                        }*/
                        // TODO Diagnose
                    }

                    if (!advance(escapedLines))
                        return;
                }
            }

            TokenType Lexer::lexNumber(uint32_t& escapedLines, uint32_t base)
            {
                TokenType tokenType = TokenType::IntegerLiteral;

                // At the start of things, we just concern ourselves with digits
                lexDigits(escapedLines, base);

                if (peek() == '.')
                {
                    if (!advance(escapedLines))
                        return TokenType::FloatingPointLiteral;

                    lexNumberAfterDecimalPoint(escapedLines, base);
                    return TokenType::FloatingPointLiteral;
                }

                if (maybeLexNumberExponent(escapedLines, base))
                    tokenType = TokenType::FloatingPointLiteral;

                lexNumberSuffix(escapedLines);
                return tokenType;
            }

            void Lexer::lexNumberAfterDecimalPoint(uint32_t& escapedLines, uint32_t base)
            {
                lexDigits(escapedLines, base);
                maybeLexNumberExponent(escapedLines, base);
                lexNumberSuffix(escapedLines);
            }

            bool Lexer::maybeLexNumberExponent(uint32_t& escapedLines, uint32_t base)
            {
                if (!isNumberExponent(peek(), base))
                    return false;

                // we saw an exponent marker
                if (advance(escapedLines))
                {
                    // TODO Diagnose
                    return true;
                }

                // Now start to read the exponent
                switch (peek())
                {
                    case '+':
                    case '-':
                        if (advance(escapedLines))
                        {
                            // TODO Diagnose
                            return true;
                        }
                        break;
                }

                // TODO(tfoley): it would be an error to not see digits here...
                lexDigits(escapedLines, 10);

                return true;
            }

            void Lexer::lexIdentifier(uint32_t& escapedLines)
            {
                for (;;)
                {
                    const auto ch = peek();

                    if (('a' <= ch) && (ch <= 'z') || ('A' <= ch) && (ch <= 'Z') || ('0' <= ch) && (ch <= '9') || (ch == '_'))
                    {
                        if (!advance(escapedLines))
                            return;

                        continue;
                    }

                    return;
                }
            }

            void Lexer::lexStringLiteralBody(uint32_t& escapedLines, U8Char quote)
            {
                bool diagnostic = true;

                // Every eof during lexing will produce diagnostic message
                ON_SCOPE_EXIT({ ASSERT(!diagnostic) });

                for (;;)
                {
                    const auto ch = peek();
                    if (ch == quote)
                    {
                        advance(escapedLines);
                        break;
                    }

                    switch (ch)
                    {
                        case '\n':
                        case '\r':
                            /*   if (auto sink = lexer->getDiagnosticSink())
                            {
                                sink->diagnose(_getSourceLoc(lexer), LexerDiagnostics::newlineInLiteral);
                            }*/
                            return;

                        case '\\': // Need to handle various escape sequence cases
                            if (!advance(escapedLines))
                                return;

                            switch (peek())
                            {
                                case '\'':
                                case '\"':
                                case '\\':
                                case '?':
                                case 'a':
                                case 'b':
                                case 'f':
                                case 'n':
                                case 'r':
                                case 't':
                                case 'v':
                                    if (!advance(escapedLines))
                                        return;
                                    break;

                                    // clang-format off
                                case '0': case '1': case '2': case '3':
                                case '4': case '5': case '6': case '7': // clang-format on
                                    // octal escape: up to 3 characters
                                    if (!advance(escapedLines))
                                        return;

                                    for (int ii = 0; ii < 3; ++ii)
                                    {
                                        int d = peek();
                                        if (('0' <= d) && (d <= '7'))
                                        {
                                            if (!advance(escapedLines))
                                                return;

                                            continue;
                                        }
                                        else
                                            break;
                                    }
                                    break;

                                case 'x':
                                    // hexadecimal escape: any number of characters
                                    if (!advance(escapedLines))
                                        return;

                                    for (;;)
                                    {
                                        int d = peek();
                                        if (('0' <= d) && (d <= '9') || ('a' <= d) && (d <= 'f') || ('A' <= d) && (d <= 'F'))
                                        {
                                            if (!advance(escapedLines))
                                                return;

                                            continue;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    break;
                                    // TODO: Unicode escape sequences
                            }
                            break;

                        default:
                            if (!advance(escapedLines))
                            {
                                // TODO Diagnostic
                                // sink->diagnose(_getSourceLoc(lexer), LexerDiagnostics::endOfFileInLiteral);
                                return;
                            }
                            continue;
                    }
                }

                // If we are reached this point. We didn't occur any eof. Disable diagnosting message.
                diagnostic = false;
            }

            bool Lexer::advance(uint32_t& escapedLines)
            {
                ASSERT(!isReachEOF());

                cursor_++;

                if (!isReachEOF() && peek() == '\\')
                {
                    if (checkForEscapedNewline(cursor_, end_))
                    {
                        escapedLines++;
                        handleEscapedNewline(cursor_, end_);
                    }
                }

                return !isReachEOF();
            }
        }
    }
}