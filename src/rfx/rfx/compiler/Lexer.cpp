#include "Lexer.hpp"

#include "rfx/compiler/DiagnosticCore.hpp"
#include "compiler/CompileContext.hpp"
#include "common/LinearAllocator.hpp"

#include <iterator>
#include <unordered_map>

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            inline bool isWhiteSpace(U8Glyph ch) { return (ch == ' ' || ch == '\t'); }
            inline bool isNewLineChar(U8Glyph ch) { return (ch == '\n' || ch == '\r'); }
            inline bool isEOF(U8Glyph ch) { return ch == Lexer::kEOF; }

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

            char* appendGlyph(U8Glyph cp, char* result)
            {
                if (cp <= 0xFF) // one octet
                    *(result++) = static_cast<uint8_t>(cp);
                else if (cp <= 0xFFFF)
                { // two octets
                    *(result++) = static_cast<uint8_t>(cp >> 8);
                    *(result++) = static_cast<uint8_t>(cp & 0xFF);
                }
                else if (cp <= 0xFFFFFF)
                { // three octets
                    *(result++) = static_cast<uint8_t>(cp >> 16);
                    *(result++) = static_cast<uint8_t>((cp >> 8) & 0xFF);
                    *(result++) = static_cast<uint8_t>(cp & 0xFF);
                }
                else
                { // four octets
                    *(result++) = static_cast<uint8_t>(cp >> 24);
                    *(result++) = static_cast<uint8_t>((cp >> 16) & 0xFF);
                    *(result++) = static_cast<uint8_t>((cp >> 8) & 0xFF);
                    *(result++) = static_cast<uint8_t>(cp & 0xFF);
                }
                return result;
            }

            char* scrubbingToken(const U8Char* srcBegin, const U8Char* srcEnd, U8Char* dstBegin, bool scrubbingEscapedCharacters)
            {
                auto cursor = srcBegin;
                auto dst = dstBegin;

                while (cursor != srcEnd)
                {
                    if (*cursor == '\\')
                    {
                        if (checkForEscapedNewline(cursor, srcEnd))
                        {
                            cursor++;
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

                        if (scrubbingEscapedCharacters)
                        {
                            const auto ch = *++cursor;

                            switch (ch)
                            {
                                // clang-format off
                                // Simple characters that just needed to be escaped
                                case '\'': case '\"':
                                case '\\': case '?':
                                    *dst++ = *cursor++;
                                    continue;

                                // Traditional escape sequences for special characters
                                case 'a': *dst++ = '\a'; cursor++; continue;
                                case 'b': *dst++ = '\b'; cursor++; continue;
                                case 'f': *dst++ = '\f'; cursor++; continue;
                                case 'n': *dst++ = '\n'; cursor++; continue;
                                case 'r': *dst++ = '\r'; cursor++; continue;
                                case 't': *dst++ = '\t'; cursor++; continue;
                                case 'v': *dst++ = '\v'; cursor++; continue;

                                // Octal escape: up to 3 characterws
                                case '0': case '1': case '2':
                                case '3': case '4': case '5':
                                case '6': case '7': // clang-format on
                                {
                                    int value = 0;
                                    for (int ii = 0; ii < 3; ++ii)
                                    {
                                        char d = *cursor;
                                        if (('0' <= d) && (d <= '7'))
                                        {
                                            value = value * 8 + (d - '0');
                                            cursor++;
                                            continue;
                                        }
                                        else
                                            break;
                                    }

                                    // TODO: add support for appending an arbitrary code point?
                                    *dst++ = (char)value;;
                                    continue;
                                }

                                // Hexadecimal escape: any number of characters
                                case 'x':
                                {
                                    U8Glyph value = 0;
                                    for (;;)
                                    {
                                        char d = *++cursor;
                                        U8Glyph digitValue = 0;
                                        if (('0' <= d) && (d <= '9'))
                                        {
                                            digitValue = d - '0';
                                        }
                                        else if (('a' <= d) && (d <= 'f'))
                                        {
                                            digitValue = 10 + d - 'a';
                                        }
                                        else if (('A' <= d) && (d <= 'F'))
                                        {
                                            digitValue = 10 + d - 'A';
                                        }
                                        else
                                            break;

                                        value = value * 16 + digitValue;
                                    }

                                    dst = appendGlyph(value, dst);
                                    continue;
                                }
                            }
                        }
                    }
                    *dst++ = *cursor++;
                }

                return dst;
            }

            template <typename... Args>
            inline void diagnose(Lexer* lexer, const SourceLocation& location, const HumaneSourceLocation& humaneSourceLocation, const DiagnosticInfo& info, Args&&... args)
            {
                ASSERT(lexer);

                if (Common::IsSet(lexer->GetLexerFlags(), Lexer::Flags::SuppressDiagnostics))
                    return;

                lexer->GetDiagnosticSink().Diagnose(location, humaneSourceLocation, info, args...);
            }
        }

        Token TokenReader::AdvanceToken()
        {
            Token result = nextToken_;

            if (cursor_ != end_)
                cursor_++;

            updateLookaheadToken();
            return result;
        }

        void TokenReader::updateLookaheadToken()
        {
            // We assume here that we can read a token from a non-null `cursor_`
            // *even* in the case where `cursor_ == end_`, because the invariant
            // for lists of tokens is that they should be terminated with and
            // end-of-file token, so that there is always a token "one past the end."
            nextToken_ = *cursor_;

            // If the token we read came from the end of the sub-sequence we are
            // reading, then we will change the token type to an end-of-file token
            // so that code that reads from the sequence and expects a terminating
            // EOF will find it.
            //
            // TODO: We might eventually want a way to look at the actual token type
            // and not just use EOF in all cases: e.g., when emitting diagnostic
            // messages that include the token that is seen.
            if (cursor_ == end_)
                nextToken_.type = Token::Type::EndOfFile;
        }

        Lexer::Lexer(const std::shared_ptr<SourceView>& sourceView,
                     const std::shared_ptr<CompileContext>& context)
            : sourceView_(sourceView),
              context_(context)
        {
            ASSERT(sourceView)
            ASSERT(context)

            auto content = sourceView->GetContent();

            if (sourceView_->GetPathInfo().type == PathInfo::Type::Split)
            {
                linesCounter_ = sourceView_->GetInitiatingHumaneLocation().line;
                columnCounter_ = sourceView_->GetInitiatingHumaneLocation().column;
            }

            begin_ = content.Begin();
            cursor_ = begin_;
            end_ = content.End();
        }

        Lexer::~Lexer() { }

        Common::LinearAllocator& Lexer::getAllocator() { return context_->allocator; }
        DiagnosticSink& Lexer::GetDiagnosticSink() const { return context_->sink; }

        Token Lexer::ReadToken()
        {
            const auto& sourceLocation = getSourceLocation();
            const auto& humaneLocation = getHumaneSourceLocation();

            if (isReachEOF())
            {
                const auto tokenSlice = UnownedStringSlice(nullptr, nullptr);
                return Token(Token::Type::EndOfFile, tokenSlice, sourceLocation, humaneLocation);
            }

            const auto tokenBegin = cursor_;
            const auto tokenType = scanToken();
            const auto tokenEnd = cursor_;

            // The flags on the token we just lexed will be based
            // on the current state of the lexer.
            auto tokenFlags = tokenflags_;
            auto tokenSlice = UnownedStringSlice(tokenBegin, tokenEnd);

            if (Common::IsAny(tokenflags_, Token::Flags::EscapedNewLines | Token::Flags::EscapedCharacters))
            {
                // "scrubbing" token value here to remove escaped newlines...
                // Only perform this work if we encountered an escaped newline while lexing this token
                // Allocate space that will always be more than enough for stripped contents
                const size_t allocationSize = std::distance(tokenBegin, tokenEnd);

                const auto dstBegin = (char*)getAllocator().Allocate(allocationSize);
                const auto dstEnd = scrubbingToken(tokenBegin, tokenEnd, dstBegin, Common::IsSet(tokenflags_, Token::Flags::EscapedCharacters));
                tokenSlice = UnownedStringSlice(dstBegin, dstEnd);

                // Reset flags
                tokenflags_ &= ~Token::Flags::EscapedNewLines & ~Token::Flags::EscapedCharacters;
            }

            switch (tokenType)
            {
                case Token::Type::NewLine:
                {
                    // If we just reached the end of a line, then the next token
                    // should count as being at the start of a line, and also after
                    // whitespace.
                    tokenflags_ = Token::Flags::AtStartOfLine | Token::Flags::AfterWhitespace;
                    break;
                }
                case Token::Type::StringLiteral:
                case Token::Type::CharLiteral:
                    // Trim quotings
                    tokenSlice = UnownedStringSlice(tokenSlice.Begin() + 1, tokenSlice.End() - 1);
                    break;
                case Token::Type::WhiteSpace:
                case Token::Type::BlockComment:
                case Token::Type::LineComment:
                {
                    // True horizontal whitespace and comments both count as whitespace.
                    //
                    // Note that a line comment does not include the terminating newline,
                    // we do not need to set `AtStartOfLine` here.
                    tokenflags_ |= Token::Flags::AfterWhitespace;
                    break;
                }
                default:
                {
                    // If we read some token other then the above cases, then we are
                    // neither after whitespace nor at the start of a line.
                    tokenflags_ = Token::Flags::None;
                    break;
                }
            }

            return Token(tokenType, tokenSlice, sourceLocation, humaneLocation, tokenFlags);
        }

        TokenList Lexer::LexAllSemanticTokens()
        {
            TokenList tokenList;

            for (;;)
            {
                const auto& token = ReadToken();

                switch (token.type)
                {
                    default:
                        break;

                    case Token::Type::WhiteSpace:
                    case Token::Type::BlockComment:
                    case Token::Type::LineComment:
                    case Token::Type::NewLine:
                        continue;
                }

                tokenList.push_back(token);

                if (token.type == Token::Type::EndOfFile)
                    return tokenList;
            }
        }

        Token::Type Lexer::scanToken()
        {
            ASSERT(!isReachEOF())

            switch (peek())
            {
                default: break;

                case '\r':
                case '\n':
                {
                    handleNewlineSequence();
                    return Token::Type::NewLine;
                }

                case ' ':
                case '\t':
                {
                    handleWhiteSpace();
                    return Token::Type::WhiteSpace;
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
                            return Token::Type::FloatingPointLiteral;

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
                                    return Token::Type::Ellipsis;

                                default:
                                    return Token::Type::DotDot;
                            }

                        default:
                            return Token::Type::Dot;
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
                            return Token::Type::IntegerLiteral;

                        case '.':
                            advance();
                            lexNumberAfterDecimalPoint(10);
                            return Token::Type::FloatingPointLiteral;

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
                            diagnose(this, loc, humaneLoc, LexerDiagnostics::octalLiteral);
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
                    return Token::Type::Identifier;

                case '\"':
                    advance();
                    lexStringLiteralBody('\"');
                    return Token::Type::StringLiteral;

                case '\'':
                    advance();
                    lexStringLiteralBody('\'');
                    return Token::Type::CharLiteral;

                case '+':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '+': advance(); return Token::Type::OpInc;
                        case '=': advance(); return Token::Type::OpAddAssign;
                        default: return Token::Type::OpAdd;
                    } // clang-format on

                case '-':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '-': advance(); return Token::Type::OpDec;
                        case '=': advance(); return Token::Type::OpSubAssign;
                        case '>': advance(); return Token::Type::RightArrow;
                        default: return Token::Type::OpSub;
                    } // clang-format on

                case '*':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpMulAssign;
                        default: return Token::Type::OpMul;
                    } // clang-format on

                case '/':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpDivAssign;
                        case '/': handleLineComment(); return Token::Type::LineComment;
                        case '*': handleBlockComment(); return Token::Type::BlockComment;
                        default: return Token::Type::OpDiv;
                    } // clang-format on

                case '%':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpModAssign;
                        default: return Token::Type::OpMod;
                    } // clang-format on

                case '|':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '|': advance(); return Token::Type::OpOr;
                        case '=': advance(); return Token::Type::OpOrAssign;
                        default: return Token::Type::OpBitOr;
                    } // clang-format on

                case '&':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '&': advance(); return Token::Type::OpAnd;
                        case '=': advance(); return Token::Type::OpAndAssign;
                        default: return Token::Type::OpBitAnd;
                    } // clang-format on

                case '^':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpXorAssign;
                        default: return Token::Type::OpBitXor;
                    } // clang-format on

                case '>':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '>':
                            advance();
                            switch (peek())
                            {
                                case '=': advance(); return Token::Type::OpShrAssign;
                                default: return Token::Type::OpRsh;
                            }

                        case '=': advance(); return Token::Type::OpGeq;
                        default: return Token::Type::OpGreater;
                    } // clang-format on

                case '<':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '<':
                            advance();
                            switch (peek())
                            {
                                case '=': advance(); return Token::Type::OpShlAssign;
                                default: return Token::Type::OpLsh;
                            }
                        case '=': advance(); return Token::Type::OpLeq;
                        default: return Token::Type::OpLess;
                    } // clang-format on

                case '=':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpEql;
                        default: return Token::Type::OpAssign;
                    } // clang-format on

                case '!':
                    advance();
                    switch (peek())
                    { // clang-format off
                        case '=': advance(); return Token::Type::OpNeq;
                        default: return Token::Type::OpNot;
                    } // clang-format on

                case '#':
                    advance();

                    switch (peek())
                    { // clang-format off
                        case '#': advance(); return Token::Type::PoundPound;
                        default: return Token::Type::Pound;
                    } // clang-format on

                case '~':
                    advance();
                    return Token::Type::OpBitNot;

                case ':':
                {
                    advance();
                    if (peek() == ':')
                    {
                        advance();
                        return Token::Type::Scope;
                    }
                    return Token::Type::Colon;
                }
                // clang-format off
                case ';': advance(); return Token::Type::Semicolon;
                case ',': advance(); return Token::Type::Comma;

                case '{': advance(); return Token::Type::LBrace;
                case '}': advance(); return Token::Type::RBrace;
                case '[': advance(); return Token::Type::LBracket;
                case ']': advance(); return Token::Type::RBracket;
                case '(': advance(); return Token::Type::LParent;
                case ')': advance(); return Token::Type::RParent;

                case '?': advance(); return Token::Type::QuestionMark;
                case '@': advance(); return Token::Type::At;
                case '$': advance(); return Token::Type::Dollar; // clang-format on
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

                        diagnose(this, loc, humaneLoc, LexerDiagnostics::illegalCharacterPrint, charString);
                    }
                    else
                    {
                        // Fallback: print as hexadecimal
                        diagnose(this, loc, humaneLoc, LexerDiagnostics::illegalCharacterHex, uint32_t(ch));
                    }
                }

                for (;;)
                {
                    advance();
                    const auto ch = peek();

                    if (isWhiteSpace(ch) || isNewLineChar(ch) || isEOF(ch))
                        break;
                }

                return Token::Type::InvalidCharacter;
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
                { // clang-format off
                    case kEOF:
                        diagnose(this, getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::endOfFileInBlockComment);
                        return;

                    case '\r': case '\n':
                        handleNewlineSequence(); 
                        continue;

                    case '*':
                        advance();
                        switch (peek())
                        {
                            case '/': advance(); return;
                            default: continue;
                        }

                    default: advance(); continue;
                } // clang-format on
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

            tokenflags_ |= Token::Flags::EscapedNewLines;

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
                    diagnose(this, getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::invalidDigitForBase, charString, base);
                }

                advance();
            }
        }

        Token::Type Lexer::lexNumber(uint32_t base)
        {
            Token::Type tokenType = Token::Type::IntegerLiteral;

            // At the start of things, we just concern ourselves with digits
            lexDigits(base);

            if (peek() == '.')
            {
                advance();
                lexNumberAfterDecimalPoint(base);
                return Token::Type::FloatingPointLiteral;
            }

            if (maybeLexNumberExponent(base))
                tokenType = Token::Type::FloatingPointLiteral;

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
                        diagnose(this, getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::endOfFileInLiteral);
                        return;

                    case '\n':
                    case '\r':
                        diagnose(this, getSourceLocation(), getHumaneSourceLocation(), LexerDiagnostics::newlineInLiteral);
                        return;

                    case '\\': // Need to handle various escape sequence cases
                        advance();

                        tokenflags_ |= Token::Flags::EscapedCharacters;
                        switch (peek())
                        {
                            // clang-format off
                            case '\'': case '\"': case '\\': case '?':
                            case 'a':  case 'b':  case 'f':  case 'n':
                            case 'r':  case 't':  case 'v': // clang-format on
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
                                        break;
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

            // TODO: Configure tab intent
            const uint32_t intent = (*cursor_ == '\t') ? 4 : 1;
            columnCounter_.Increment(intent);

            utf8::next(cursor_, end_);

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
    }
}