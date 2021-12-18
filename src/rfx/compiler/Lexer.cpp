#include "Lexer.hpp"

#include "compiler/DiagnosticCore.hpp"

#include "common/LinearAllocator.hpp"

#include <iterator>
#include <unordered_map>

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            namespace
            {
                inline bool isWhiteSpace(U8Glyph ch)
                {
                    return (ch == ' ' || ch == '\t');
                }

                inline bool isNewLineChar(U8Glyph ch)
                {
                    return (ch == '\n' || ch == '\r');
                }

                inline bool isEOF(U8Glyph ch)
                {
                    return ch == Lexer::kEOF;
                }

                bool isNumberExponent(U8Glyph ch, uint32_t base)
                {
                    switch (ch)
                    { // clang-format off
                        case 'e': case 'E': return (base == 10);
                        case 'p': case 'P': return (base == 16);
                        default: return false;
                    } // clang-format on
                }

                bool checkForEscapedNewline(const U8Char* cursor, const U8Char* end)
                {
                    ASSERT(*cursor == '\\')

                    U8Glyph next = 0;

                    // Peak next char if exist
                    if (std::distance(cursor, end) > 1)
                        next = *(cursor + 1);

                    return isNewLineChar(next);
                }

                char* scrubbingToken(const U8Char* srcBegin, const U8Char* srcEnd, U8Char* dstBegin)
                {
                    auto cursor = srcBegin;
                    auto dst = dstBegin;

                    while (cursor != srcEnd)
                    {
                        if (*cursor == '\\')
                        {
                            if (checkForEscapedNewline(cursor, srcEnd))
                            {
                                const auto first = *cursor;

                                if (++cursor == srcEnd)
                                    return dst;

                                const auto second = *cursor;

                                // Handle all newline sequences
                                //  "\n"
                                //  "\r"
                                //  "\r\n"
                                //  "\n\r"
                                if (isNewLineChar(second) && first != second)
                                    cursor++;

                                continue;
                            }
                        }
                        *dst++ = *cursor++;
                    }

                    return dst;
                }
            }

            Lexer::Lexer(const std::shared_ptr<SourceView>& sourceView, const std::shared_ptr<DiagnosticSink>& diagnosticSink)
                : allocator_(new LinearAllocator(1024)),
                  sourceView_(sourceView),
                  sink_(diagnosticSink)
            {
                ASSERT(sourceView)
                ASSERT(diagnosticSink)

                auto content = sourceView->GetContent();

                begin_ = content.Begin();
                cursor_ = begin_;
                end_ = content.End();
            }

            Lexer::~Lexer()
            {
            }

            Token Lexer::GetNextToken()
            {
                const auto& SourceLocation = getSourceLocation();

                if (isReachEOF())
                {
                    const auto tokenSlice = UnownedStringSlice(nullptr, nullptr);
                    return Token(TokenType::EndOfFile, tokenSlice, SourceLocation);
                }

                const auto tokenBegin = cursor_;
                const auto tokenType = scanToken();
                const auto tokenEnd = cursor_;

                if ((flags_ & Flags::EscapedNewLines) == Flags::EscapedNewLines)
                {
                    // Reset flag
                    flags_ &= ~Flags::EscapedNewLines;

                    // "scrubbing" token value here to remove escaped newlines...
                    // Only perform this work if we encountered an escaped newline while lexing this token
                    // Allocate space that will always be more than enough for stripped contents
                    const size_t allocationSize = std::distance(tokenBegin, tokenEnd);

                    const auto dstBegin = (char*)allocator_->Allocate(allocationSize);
                    const auto dstEnd = scrubbingToken(tokenBegin, tokenEnd, dstBegin);

                    const auto tokenSlice = UnownedStringSlice(dstBegin, dstEnd);

                    return Token(tokenType, tokenSlice, SourceLocation);
                }

                switch (tokenType)
                {
                    case TokenType::NewLine:
                    {
                        // If we just reached the end of a line, then the next token
                        // should count as being at the start of a line, and also after
                        // whitespace.
                        flags_ = Flags::AtStartOfLine | Flags::AfterWhitespace;
                        break;
                    }
                    case TokenType::WhiteSpace:
                    case TokenType::BlockComment:
                    case TokenType::LineComment:
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
                }

                const auto tokenSlice = UnownedStringSlice(tokenBegin, tokenEnd);
                return Token(tokenType, tokenSlice, SourceLocation);
            }

            std::shared_ptr<std::vector<Token>> Lexer::LexAllSemanticTokens()
            {
                auto tokenList = std::make_shared<std::vector<Token>>();

                for (;;)
                {
                    const auto& token = GetNextToken();

                    switch (token.type)
                    {
                        default:
                            break;

                        case TokenType::WhiteSpace:
                        case TokenType::BlockComment:
                        case TokenType::LineComment:
                        case TokenType::NewLine:
                            continue;
                    }

                    tokenList->push_back(token);

                    if (token.type == TokenType::EndOfFile)
                        return tokenList;
                }
            }

            TokenType Lexer::scanToken()
            {
                ASSERT(!isReachEOF())

                switch (peek())
                {
                    default:
                        break;

                    case '\r':
                    case '\n':
                    {
                        handleNewlineSequence();
                        return TokenType::NewLine;
                    }

                    case ' ':
                    case '\t':
                    {
                        handleWhiteSpace();
                        return TokenType::WhiteSpace;
                    }

                    case '.':
                    {
                        advance();

                        switch (peek())
                        {
                            // clang-format off
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9': // clang-format on
                                lexNumberAfterDecimalPoint(10);
                                return TokenType::FloatingPointLiteral;

                            case '.':
                                // Note: consuming the second `.` here means that
                                // we cannot back up and return a `.` token by itself
                                // any more. We thus end up having distinct tokens for
                                // `.`, `..`, and `...` even though the `..` case is
                                // not part of HLSL.
                                //
                                advance();
                                switch (peek())
                                {
                                    case '.':
                                        advance();
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
                        return lexNumber(10);

                    case '0':
                    {
                        const auto& loc = getSourceLocation();
                        const auto& humaneLoc = getHumaneSourceLocation();

                        advance();

                        switch (peek())
                        {
                            default:
                                lexNumberSuffix();
                                return TokenType::IntegerLiteral;

                            case '.':
                                advance();
                                lexNumberAfterDecimalPoint(10);
                                return TokenType::FloatingPointLiteral;

                            case 'x':
                            case 'X':
                                advance();
                                return lexNumber(16);

                            case 'b':
                            case 'B':
                                advance();
                                return lexNumber(2);

                            // clang-format off
                            case '0': case '1': case '2': case '3': case '4': 
                            case '5': case '6': case '7': case '8': case '9': // clang-format on
                                sink_->Diagnose(loc, humaneLoc, LexerDiagnostics::octalLiteral);
                                return lexNumber(8);
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
                        lexIdentifier();
                        return TokenType::Identifier;

                    case '\"':
                        advance();
                        lexStringLiteralBody('\"');
                        return TokenType::StringLiteral;

                    case '\'':
                        advance();
                        lexStringLiteralBody('\'');
                        return TokenType::CharLiteral;

                    case '+':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '+': advance(); return TokenType::OpInc;
                            case '=': advance(); return TokenType::OpAddAssign;
                            default: return TokenType::OpAdd; 
                        } // clang-format on

                    case '-':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '-': advance(); return TokenType::OpDec;
                            case '=': advance(); return TokenType::OpSubAssign;
                            case '>': advance(); return TokenType::RightArrow;
                            default: return TokenType::OpSub;
                        } // clang-format on

                    case '*':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpMulAssign;
                            default: return TokenType::OpMul;
                        } // clang-format on

                    case '/':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpDivAssign;
                            case '/': handleLineComment(); return TokenType::LineComment;
                            case '*': handleBlockComment(); return TokenType::BlockComment;     
                            default: return TokenType::OpDiv;
                        } // clang-format on  

                    case '%':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpModAssign;
                            default: return TokenType::OpMod;
                        } // clang-format on  

                    case '|':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '|': advance(); return TokenType::OpOr;
                            case '=': advance(); return TokenType::OpOrAssign;
                            default: return TokenType::OpBitOr;
                        } // clang-format on  

                    case '&':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '&': advance(); return TokenType::OpAnd;
                            case '=': advance(); return TokenType::OpAndAssign;
                            default: return TokenType::OpBitAnd;
                        } // clang-format on  

                    case '^':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpXorAssign;
                            default: return TokenType::OpBitXor;
                        } // clang-format on  

                    case '>':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '>':
                                advance();
                                switch (peek())
                                {
                                    case '=': advance(); return TokenType::OpShrAssign;
                                    default: return TokenType::OpRsh;
                                }

                            case '=': advance(); return TokenType::OpGeq;
                            default: return TokenType::OpGreater;
                        } // clang-format on 

                    case '<':
                       advance();
                        switch (peek())
                        { // clang-format off
                            case '<': 
                                advance();
                                switch (peek())
                                {
                                    case '=': advance(); return TokenType::OpShlAssign;
                                    default: return TokenType::OpLsh;
                                }
                            case '=': advance(); return TokenType::OpLeq;
                            default: return TokenType::OpLess;
                        } // clang-format on 

                    case '=':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpEql;
                            default: return TokenType::OpAssign;
                        } // clang-format on 

                    case '!':
                        advance();
                        switch (peek())
                        { // clang-format off
                            case '=': advance(); return TokenType::OpNeq;
                            default: return TokenType::OpNot;
                        } // clang-format on 

                    case '#':
                        // Preprocessor directives always on start the line or after whitspace
                        if ((flags_ & Flags::AtStartOfLine) != Flags::None ||
                            (flags_ & Flags::AfterWhitespace) != Flags::None)
                        {
                            return lexDirective();
                        }

                        advance();

                        switch (peek())
                        { // clang-format off
                            case '#': advance(); return TokenType::PoundPound;
                            default: return TokenType::Pound;
                        } // clang-format on 

                    case '~':
                        advance();
                        return TokenType::OpBitNot;

                    case ':':
                    {
                        advance();
                        if (peek() == ':')
                        {
                            advance();
                            return TokenType::Scope;
                        }
                        return TokenType::Colon;
                    }
                    // clang-format off
                    case ';': advance(); return TokenType::Semicolon;
                    case ',': advance(); return TokenType::Comma;

                    case '{': advance(); return TokenType::LBrace;
                    case '}': advance(); return TokenType::RBrace;
                    case '[': advance(); return TokenType::LBracket;
                    case ']': advance(); return TokenType::RBracket;
                    case '(': advance(); return TokenType::LParent;
                    case ')': advance(); return TokenType::RParent;

                    case '?': advance(); return TokenType::QuestionMark;
                    case '@': advance(); return TokenType::At;
                    case '$': advance(); return TokenType::Dollar; // clang-format on
                }

                // TODO(tfoley): If we ever wanted to support proper Unicode
                // in identifiers, etc., then this would be the right place
                // to perform a more expensive dispatch based on the actual
                // code point (and not just the first byte).

                {
                    // If none of the above cases matched, then we have an
                    // unexpected/invalid character.

                    auto loc = getSourceLocation();
                    auto humaneLoc = getHumaneSourceLocation();
                    {
                        const auto ch = peek();

                        if (ch >= 0x20)
                        {
                            U8String charString;
                            utf8::append(ch, charString);

                            sink_->Diagnose(loc, humaneLoc, LexerDiagnostics::illegalCharacterPrint, charString);
                        }
                        else
                        {
                            // Fallback: print as hexadecimal
                            sink_->Diagnose(loc, humaneLoc, LexerDiagnostics::illegalCharacterHex, uint32_t(ch));
                        }
                    }

                    for (;;)
                    {
                        advance();
                        const auto ch = peek();

                        if (isWhiteSpace(ch) || isNewLineChar(ch) || isEOF(ch))
                            break;
                    }

                    return TokenType::Invalid;
                }
            }

            void Lexer::handleWhiteSpace()
            {
                ASSERT(isWhiteSpace(peek()))

                for (;;)
                {
                    advance();

                    if (!isWhiteSpace(peek()))
                        break;
                }
            }

            void Lexer::handleLineComment()
            {
                ASSERT(peek() == '/')

                for (;;)
                {
                    advance();

                    if (isNewLineChar(peek()) || isEOF(peek()))
                        break;
                }
            }

            void Lexer::handleBlockComment()
            {
                ASSERT(peek() == '*')

                for (;;)
                {
                    switch (peek())
                    {
                        case kEOF:
                            sink_->Diagnose(getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::endOfFileInBlockComment);
                            return;

                        case '\r':
                        case '\n':
                            handleNewlineSequence();
                            continue;

                        case '*':
                            advance();
                            switch (peek())
                            {
                                case '/':
                                    advance();
                                    return;

                                default:
                                    continue;
                            }

                        default:
                            advance();
                            continue;
                    }
                }
            }

            void Lexer::handleNewlineSequence()
            {
                ASSERT(isNewLineChar(*cursor_))

                const auto first = peek();

                advance();

                columnCounter_.Reset(1);
                linesCounter_.Increment();

                const auto second = peek();

                if (second == kEOF)
                    return;

                // Handle all newline sequences
                //  "\n"
                //  "\r"
                //  "\r\n"
                //  "\n\r"
                if (isNewLineChar(second) && first != second)
                    advance();

                columnCounter_.Reset(1);
            }

            void Lexer::handleEscapedNewline()
            {
                ASSERT(checkForEscapedNewline(cursor_, end_));

                flags_ |= Flags::EscapedNewLines;

                advance();
                handleNewlineSequence();
            }

            void Lexer::lexNumberSuffix()
            {
                // Be liberal in what we accept here, so that figuring out
                // the semantics of a numeric suffix is left up to the parser
                // and semantic checking logic.
                for (;;)
                {
                    U8Glyph ch = peek();

                    // Accept any alphanumeric character, plus underscores.
                    if ((('a' <= ch) && (ch <= 'z')) ||
                        (('A' <= ch) && (ch <= 'Z')) ||
                        (('0' <= ch) && (ch <= '9')) ||
                        (ch == '_'))
                    {
                        advance();
                        continue;
                    }

                    // Stop at the first character that isn't
                    // alphanumeric.
                    return;
                }
            }

            void Lexer::lexDigits(uint32_t base)
            {
                for (;;)
                {
                    U8Glyph ch = peek();

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
                    {
                        U8String charString;
                        utf8::append(ch, charString);
                        sink_->Diagnose(getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::invalidDigitForBase, charString, base);
                    }

                    advance();
                }
            }

            TokenType Lexer::lexDirective()
            {
                ASSERT(peek() == '#')

                advance();

                for (;;)
                {
                    const auto ch = peek();
                    switch (ch)
                    {
                        case ' ':
                        case '\t':
                            handleWhiteSpace();
                            continue;
                        case '\r':
                        case '\n':
                            handleNewlineSequence();
                            return TokenType::NullDirective;

                        default:
                        {
                            const auto loc = getSourceLocation();

                            const auto begin = cursor_;
                            lexIdentifier();
                            const auto end = cursor_;

                            const auto identifier = U8String(begin, end);
                            return getDirectiveTokenFromName(loc, identifier);
                        }
                    }
                }
            }

            TokenType Lexer::lexNumber(uint32_t base)
            {
                TokenType tokenType = TokenType::IntegerLiteral;

                // At the start of things, we just concern ourselves with digits
                lexDigits(base);

                if (peek() == '.')
                {
                    advance();
                    lexNumberAfterDecimalPoint(base);
                    return TokenType::FloatingPointLiteral;
                }

                if (maybeLexNumberExponent(base))
                    tokenType = TokenType::FloatingPointLiteral;

                lexNumberSuffix();
                return tokenType;
            }

            void Lexer::lexNumberAfterDecimalPoint(uint32_t base)
            {
                lexDigits(base);
                maybeLexNumberExponent(base);
                lexNumberSuffix();
            }

            bool Lexer::maybeLexNumberExponent(uint32_t base)
            {
                if (!isNumberExponent(peek(), base))
                    return false;

                // we saw an exponent marker
                advance();

                // Now start to read the exponent
                switch (peek())
                {
                    case '+':
                    case '-':
                        advance();
                        break;
                }

                // TODO(tfoley): it would be an error to not see digits here...
                lexDigits(10);

                return true;
            }

            void Lexer::lexIdentifier()
            {
                for (;;)
                {
                    const auto ch = peek();

                    if ((('a' <= ch) && (ch <= 'z')) ||
                        (('A' <= ch) && (ch <= 'Z')) ||
                        (('0' <= ch) && (ch <= '9')) ||
                        (ch == '_'))
                    {
                        advance();
                        continue;
                    }

                    return;
                }
            }

            void Lexer::lexStringLiteralBody(U8Glyph quote)
            {
                for (;;)
                {
                    const auto ch = peek();
                    if (ch == quote)
                    {
                        advance();
                        break;
                    }

                    switch (ch)
                    {
                        case kEOF:
                            sink_->Diagnose(getSourceLocation(),getHumaneSourceLocation(), LexerDiagnostics::endOfFileInLiteral);
                            return;

                        case '\n':
                        case '\r':
                            sink_->Diagnose(getSourceLocation(),getHumaneSourceLocation(), LexerDiagnostics::newlineInLiteral);
                            return;

                        case '\\': // Need to handle various escape sequence cases
                            advance();

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
                                    advance();
                                    break;

                                // clang-format off
                                case '0': case '1': case '2': case '3':
                                case '4': case '5': case '6': case '7': // clang-format on
                                    // octal escape: up to 3 characters
                                    advance();

                                    for (int ii = 0; ii < 3; ++ii)
                                    {
                                        int d = peek();
                                        if (('0' <= d) && (d <= '7'))
                                        {
                                            advance();
                                            continue;
                                        }
                                        else
                                            break;
                                    }
                                    break;

                                case 'x':
                                    // hexadecimal escape: any number of characters
                                    advance();

                                    for (;;)
                                    {
                                        int d = peek();
                                        if ((('0' <= d) && (d <= '9')) ||
                                            (('a' <= d) && (d <= 'f')) ||
                                            (('A' <= d) && (d <= 'F')))
                                        {
                                            advance();
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
                            advance();
                            continue;
                    }
                }
            }

            void Lexer::advance()
            {
                ASSERT(!isReachEOF());

                utf8::next(cursor_, end_);

                columnCounter_.Increment();

                if (!isReachEOF() && peek() == '\\')
                {
                    if (checkForEscapedNewline(cursor_, end_))
                        handleEscapedNewline();
                }
            }

            SourceLocation Lexer::getSourceLocation()
            {
                return sourceView_->GetSourceLocation(std::distance(begin_, cursor_));
            }

            HumaneSourceLocation Lexer::getHumaneSourceLocation()
            {
                return HumaneSourceLocation(linesCounter_.Value(), columnCounter_.Value());
            }

            TokenType Lexer::getDirectiveTokenFromName(const SourceLocation& location, const U8String& name)
            {
                static const std::unordered_map<U8String, TokenType> directives = {
                    { "if", TokenType::IfDirective },
                    { "ifdef", TokenType::IfDefDirective },
                    { "ifndef", TokenType::IfNDefDirective },
                    { "else", TokenType::ElseDirective },
                    { "elif", TokenType::ElifDirective },
                    { "endif", TokenType::EndIfDirective },
                    { "include", TokenType::IncludeDirective },
                    { "define", TokenType::DefineDirective },
                    { "undef", TokenType::UndefDirective },
                    { "warning", TokenType::WarningDirective },
                    { "error", TokenType::ErrorDirective },
                    { "line", TokenType::LineDirective },
                    { "pragma", TokenType::PragmaDirective },
                    { "", TokenType::NullDirective }
                };

                auto search = directives.find(name);
                if (search == directives.end())
                {
                    sink_->Diagnose(location, getHumaneSourceLocation(), LexerDiagnostics::unknownDirective, name);
                    return TokenType::Unknown;
                }

                return search->second;
            }
        }
    }
}