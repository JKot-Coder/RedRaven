#include "LinePreprocessor.hpp"

#include "compiler/DiagnosticCore.hpp"
#include "compiler/Lexer.hpp"

#include "core/IncludeSystem.hpp"
#include "core/StringEscapeUtil.hpp"

#include "common/LinearAllocator.hpp"
#include "common/Result.hpp"

#include <filesystem>
#include <unordered_map>

namespace RR::Rfx
{
    namespace
    {
        U8String getStringLiteralTokenValue(const Token& token)
        {
            ASSERT(token.type == Token::Type::StringLiteral || token.type == Token::Type::CharLiteral);

            const UnownedStringSlice content = token.stringSlice;

            auto cursor = utf8::iterator<const char*>(content.Begin(), content.Begin(), content.End());
            auto end = utf8::iterator<const char*>(content.End(), content.Begin(), content.End());

            auto quote = *cursor++;
            ASSERT(quote == '\'' || quote == '"');

            U8String result;
            for (;;)
            {
                ASSERT(cursor != end);

                char32_t ch = *cursor++;

                // If we see a closing quote, then we are at the end of the string literal
                if (ch == quote)
                {
                    ASSERT(cursor == end);
                    return result;
                }

                // Characters that don't being escape sequences are easy;
                // just append them to the buffer and move on.
                if (ch != '\\')
                {
                    utf8::append(ch, result);
                    continue;
                }

                // Now we look at another character to figure out the kind of
                // escape sequence we are dealing with:
                char32_t d = *cursor++;

                switch (d)
                {
                    // Simple characters that just needed to be escaped
                    case '\'':
                    case '\"':
                    case '\\':
                    case '?':
                        utf8::append(d, result);
                        continue;

                    // Traditional escape sequences for special characters
                    // clang-format off
                    case 'a': utf8::append('\a', result); continue;
                    case 'b': utf8::append('\b', result); continue;
                    case 'f': utf8::append('\f', result); continue;
                    case 'n': utf8::append('\n', result); continue;
                    case 'r': utf8::append('\r', result); continue;
                    case 't': utf8::append('\t', result); continue;
                    case 'v': utf8::append('\v', result); continue;
                    // clang-format on

                    // clang-format off
                    // Octal escape: up to 3 characterws
                    case '0': case '1': case '2': case '3': 
                    case '4': case '5': case '6': case '7':
                    { // clang-format on
                        cursor--;
                        int value = 0;
                        for (int ii = 0; ii < 3; ++ii)
                        {
                            d = *cursor;
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
                        utf8::append(value, result);
                        continue;
                    }

                    // Hexadecimal escape: any number of characters
                    case 'x':
                    {
                        char32_t value = 0;
                        for (;;)
                        {
                            d = *cursor++;
                            int digitValue = 0;
                            if (('0' <= d) && (d <= '9'))
                            {
                                digitValue = d - '0';
                            }
                            else if (('a' <= d) && (d <= 'f'))
                            {
                                digitValue = d - 'a';
                            }
                            else if (('A' <= d) && (d <= 'F'))
                            {
                                digitValue = d - 'A';
                            }
                            else
                            {
                                cursor--;
                                break;
                            }

                            value = value * 16 + digitValue;
                        }

                        // TODO: add support for appending an arbitrary code point?
                        utf8::append(value, result);
                        continue;
                    }

                        // TODO: Unicode escape sequences
                }
            }
        }
    }

    struct InputFile;

    struct DirectiveContext
    {
    public:
        // Get the name of the directive being parsed.
        inline U8String GetName() const { return token.GetContentString(); }

    public:
        Token token;
        bool parseError = false;
    };

    class LinePreprocessorImpl final : public std::enable_shared_from_this<LinePreprocessorImpl>
    {
    public:
        LinePreprocessorImpl(const std::shared_ptr<Common::LinearAllocator>& allocator,
                             const std::shared_ptr<DiagnosticSink>& diagnosticSink);

        std::shared_ptr<DiagnosticSink> GetSink() const { return sink_; }
        std::shared_ptr<Common::LinearAllocator> GetAllocator() const { return allocator_; }

        std::vector<Token> ReadAllTokens();

        // Push a new input file onto the input stack of the preprocessor
        void PushInputFile(const std::shared_ptr<InputFile>& inputFile);

    private:
        uint32_t tokenToUInt(const Token& str, int radix);

        Token readToken();

        // Pop the inner-most input file from the stack of input files
        // Force - skips the eof check
        void popInputFile(bool force = false);

        inline Token peekToken();
        inline Token::Type peekTokenType() { return peekToken().type; }

        // Read one token, with macro-expansion, without going past the end of the line.
        Token advanceToken();

        // Skip to the end of the line (useful for recovering from errors in a directive)
        void skipToEndOfLine();

        // Determine if we have read everything on the directive's line.
        bool isEndOfLine();

        bool expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic);
        bool expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken);

        void handleDirective();
        void handleLineDirective(DirectiveContext& directiveContext);

        // Helper routine to check that we find the end of a directive where we expect it.
        // Most directives do not need to call this directly, since we have
        // a catch-all case in the main `handleDirective()` function.
        // `#include` and `#line` case will call it directly to avoid complications
        // when it switches the input stream.
        void expectEndOfDirective(DirectiveContext& context);

    private:
        std::shared_ptr<DiagnosticSink> sink_;
        std::shared_ptr<Common::LinearAllocator> allocator_;

        /// A stack of "active" input files
        std::shared_ptr<InputFile> currentInputFile_;

        /// A pre-allocated token that can be returned to represent end-of-input situations.
        Token endOfFileToken_;
    };

    //
    // Input Streams
    //

    // A fundamental action in the preprocessor is to transform a stream of
    // input tokens to produce a stream of output tokens. The term "macro expansion"
    // is used to describe two inter-related transformations of this kind:
    //
    // * Given an invocation of a macro `M`, we can "play back" the tokens in the
    //   definition of `M` to produce a stream of tokens, potentially substituting
    //   in argument values for parameters, pasting tokens, etc.
    //
    // * Given an input stream, we can scan its tokens looking for macro invocations,
    //   and upon finding them expand those invocations using the first approach
    //   outlined here.
    //
    // In practice, the second kind of expansion needs to abstract over where it
    // is reading tokens from: an input file, an existing macro invocation, etc.
    // In order to support reading from streams of tokens without knowing their
    // exact implementation, we will define an abstract base class for input
    // streams.

    /// A logical stream of tokens.
    struct InputStream
    {
    public:
        InputStream(const std::weak_ptr<LinePreprocessorImpl> preprocessor)
            : preprocessor_(preprocessor)
        {
            ASSERT(preprocessor.lock());
        }

        // Because different implementations of this abstract base class will
        // store differnet amounts of data, we need a virtual descritor to
        // ensure that we can clean up after them.

        /// Clean up an input stream
        virtual ~InputStream() = default;

        // The two fundamental operations that every input stream must support
        // are reading one token from the stream, and "peeking" one token into
        // the stream to see what will be read next.

        /// Read one token from the input stream
        /// At the end of the stream should return a token with `Token::Type::EndOfFile`.
        virtual Token ReadToken() = 0;

        /// Peek at the next token in the input stream
        /// This function should return whatever `readToken()` will return next.
        /// At the end of the stream should return a token with `Token::Type::EndOfFile`.
        virtual Token PeekToken() = 0;

        // Based on `peekToken()` we can define a few more utility functions
        // for cases where we only care about certain details of the input.

        /// Peek the type of the next token in the input stream.
        Token::Type PeekTokenType() { return PeekToken().type; }

        /// Peek the location of the next token in the input stream.
        SourceLocation PeekLoc() { return PeekToken().sourceLocation; }

        /// Get the diagnostic sink to use for messages related to this stream
        std::shared_ptr<DiagnosticSink> GetSink() const
        {
            const auto preprocessor = preprocessor_.lock();
            ASSERT(preprocessor);

            return preprocessor->GetSink();
        }

        std::shared_ptr<InputStream> GetParent() const { return parent_; }

        void SetParent(const std::shared_ptr<InputStream>& parent) { parent_ = parent; }

    protected:
        /// The preprocessor that this input stream is being used by
        std::weak_ptr<LinePreprocessorImpl> preprocessor_;

        /// Parent stream in the stack of secondary input streams
        std::shared_ptr<InputStream> parent_;
    };

    // Another (relatively) simple case of an input stream is one that reads
    // tokens directly from the lexer.
    //
    // It might seem like we could simplify things even further by always lexing
    // a file into tokens first, and then using the earlier input-stream cases
    // for pre-tokenized input. The main reason we don't use that strategy is
    // that when dealing with preprocessor conditionals we will often want to
    // suppress diagnostic messages coming from the lexer when inside of disabled
    // conditional branches.
    //
    // TODO: We might be able to simplify the logic here by having the lexer buffer
    // up the issues it diagnoses along with a list of tokens, rather than diagnose
    // them directly, and then have the preprocessor or later compilation stages
    // take responsibility for actually emitting those diagnostics.

    /// An input stream that reads tokens directly using the `Lexer`
    struct LexerInputStream : InputStream
    {
    public:
        LexerInputStream() = delete;

        LexerInputStream(
            const std::weak_ptr<LinePreprocessorImpl> preprocessor, const std::shared_ptr<SourceView>& sourceView)
            : InputStream(preprocessor)
        {
            const auto sharedPreprocessor = preprocessor.lock();
            ASSERT(sharedPreprocessor);

            lexer_ = std::make_unique<Lexer>(sourceView, sharedPreprocessor->GetAllocator(), GetSink());
            lookaheadToken_ = readTokenImpl();
        }

        Lexer& GetLexer() const { return *lexer_; }

        // A common thread to many of the input stream implementations is to
        // use a single token of lookahead in order to suppor the `peekToken()`
        // operation with both simplicity and efficiency.
        Token ReadToken() override
        {
            auto result = lookaheadToken_;
            lookaheadToken_ = readTokenImpl();
            return result;
        }

        Token PeekToken() override { return lookaheadToken_; }

    private:
        /// Read a token from the lexer, bypassing lookahead
        Token readTokenImpl()
        {
            for (;;)
            {
                Token token = lexer_->ReadToken();
                switch (token.type)
                {
                    default:
                        return token;

                    case Token::Type::WhiteSpace:
                    case Token::Type::BlockComment:
                    case Token::Type::LineComment:
                        break;
                }
            }
        }

    private:
        /// The lexer state that will provide input
        std::unique_ptr<Lexer> lexer_;

        /// One token of lookahead
        Token lookaheadToken_;
    };

    // The top-level flow of the preprocessor is that it processed *input files*
    // An input file manages both the expansion of lexed tokens
    // from the source file, and also state related to preprocessor
    // directives, including skipping of code due to `#if`, etc.
    //
    // Input files are a bit like token streams, but they don't fit neatly into
    // the same abstraction due to all the special-case handling that directives
    // and conditionals require.
    struct InputFile
    {
    public:
        InputFile(const std::weak_ptr<LinePreprocessorImpl>& linePreprocessorImpl, const std::shared_ptr<SourceView>& sourceView)
        {
            ASSERT(sourceView);
            ASSERT(linePreprocessorImpl.lock());

            lexerStream_ = std::make_shared<LexerInputStream>(linePreprocessorImpl, sourceView);
        }

        /// Read one token using all the expansion and directive-handling logic
        inline Token ReadToken() { return lexerStream_->ReadToken(); }

        inline Lexer& GetLexer() const { return lexerStream_->GetLexer(); }
        inline std::shared_ptr<SourceView> GetSourceView() const { return GetLexer().GetSourceView(); }

    private:
        /// The next outer input file
        ///
        /// E.g., if this file was `#include`d from another file, then `parent_` would be
        /// the file with the `#include` directive.
        std::shared_ptr<InputFile> parent_;

        /// The lexer input stream that unexpanded tokens will be read from
        std::shared_ptr<LexerInputStream> lexerStream_;

    private:
        friend class LinePreprocessorImpl;
    };

    LinePreprocessorImpl::LinePreprocessorImpl(const std::shared_ptr<Common::LinearAllocator>& allocator,
                                               const std::shared_ptr<DiagnosticSink>& diagnosticSink)
        : sink_(diagnosticSink),
          allocator_(allocator)
    {
        ASSERT(diagnosticSink);

        endOfFileToken_.type = Token::Type::EndOfFile;
    }

    std::vector<Token> LinePreprocessorImpl::ReadAllTokens()
    {
        std::vector<Token> tokens;

        for (;;)
        {
            const auto& token = readToken();

            ASSERT(token.isValid());

            switch (token.type)
            {
                default:
                    tokens.push_back(token);
                    break;

                case Token::Type::EndOfFile:
                    // Note: we include the EOF token in the list,
                    // since that is expected by the `TokenList` type.
                    tokens.push_back(token);
                    return tokens;

                case Token::Type::WhiteSpace:
                case Token::Type::NewLine:
                case Token::Type::LineComment:
                case Token::Type::BlockComment:
                case Token::Type::Invalid:
                    break;
            }
        }
    }

    uint32_t LinePreprocessorImpl::tokenToUInt(const Token& token, int radix)
    {
        ASSERT(token.type == Token::Type::IntegerLiteral);

        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.End());
        const auto result = std::strtoul(token.stringSlice.Begin(), &end, radix);

        if (errno == ERANGE)
            GetSink()->Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "uint32_t");

        if (end == token.stringSlice.End())
            return result;

        GetSink()->Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), radix);
        return 0;
    }

    Token LinePreprocessorImpl::readToken()
    {
        for (;;)
        {
            auto inputFile = currentInputFile_;

            if (!inputFile)
                return endOfFileToken_;

            Token token = peekToken();
            if (token.type == Token::Type::EndOfFile)
            {
                popInputFile();
                continue;
            }

            // If we have a directive (`#` at start of line) then handle it
            if ((token.type == Token::Type::Pound) && Common::IsSet(token.flags, Token::Flags::AtStartOfLine))
            {
                // Parse and handle the directive
                handleDirective();
                continue;
            }

            return token;
        }
    }

    void LinePreprocessorImpl::PushInputFile(const std::shared_ptr<InputFile>& inputFile)
    {
        ASSERT(inputFile);

        inputFile->parent_ = currentInputFile_;
        currentInputFile_ = inputFile;
    }

    void LinePreprocessorImpl::popInputFile(bool force)
    {
        const auto& inputFile = currentInputFile_;
        ASSERT(inputFile);

        // We expect the file to be at its end, so that the
        // next token read would be an end-of-file token.
        Token eofToken = peekToken();

        // The #line directive forces the current input to be closed.
        // Updating the token type to close the current input correctly.
        if (force)
            eofToken.type = Token::Type::EndOfFile;

        ASSERT(eofToken.type == Token::Type::EndOfFile);

        // We will update the current file to the parent of whatever
        // the `inputFile` was (usually the file that `#include`d it).
        auto parentFile = inputFile->parent_;
        currentInputFile_ = parentFile;

        // As a subtle special case, if this is the *last* file to be popped,
        // then we will update the canonical EOF token used by the preprocessor
        // to be the EOF token for `inputFile`, so that the source location
        // information returned will be accurate.
        if (!parentFile)
            endOfFileToken_ = eofToken;
    }

    Token LinePreprocessorImpl::peekToken()
    {
        ASSERT(currentInputFile_);

        return currentInputFile_->lexerStream_->PeekToken();
    }

    Token LinePreprocessorImpl::advanceToken()
    {
        ASSERT(currentInputFile_);

        if (isEndOfLine())
            return peekToken();

        return currentInputFile_->lexerStream_->ReadToken();
    }

    void LinePreprocessorImpl::skipToEndOfLine()
    {
        while (!isEndOfLine())
            advanceToken();
    }

    bool LinePreprocessorImpl::isEndOfLine()
    {
        ASSERT(currentInputFile_);

        switch (currentInputFile_->lexerStream_->PeekTokenType())
        {
            case Token::Type::EndOfFile:
            case Token::Type::NewLine:
                return true;

            default:
                return false;
        }
    }

    bool LinePreprocessorImpl::expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic)
    {
        if (peekTokenType() != expected)
        {
            // Only report the first parse error within a directive
            if (!context.parseError)
                GetSink()->Diagnose(peekToken(), diagnostic, expected, context.GetName());

            context.parseError = true;
            return false;
        }

        advanceToken();
        return true;
    }

    void LinePreprocessorImpl::expectEndOfDirective(DirectiveContext& context)
    {
        if (!isEndOfLine())
        {
            // If we already saw a previous parse error, then don't
            // emit another one for the same directive.
            if (!context.parseError)
                GetSink()->Diagnose(peekToken(), Diagnostics::unexpectedTokensAfterDirective, context.GetName());

            skipToEndOfLine();
        }
    }

    void LinePreprocessorImpl::handleDirective()
    {
        ASSERT(peekTokenType() == Token::Type::Pound);

        // Skip the `#`
        advanceToken();

        // Create a context for parsing the directive
        DirectiveContext context;

        // Try to read the directive name.
        context.token = peekToken();

        Token::Type directiveTokenType = context.token.type;

        // An empty directive is allowed, and ignored.
        switch (directiveTokenType)
        {
            case Token::Type::EndOfFile:
            case Token::Type::NewLine:
                return;

            default:
                break;
        }

        // Otherwise the directive name had better be an identifier
        if (directiveTokenType != Token::Type::Identifier)
        {
            GetSink()->Diagnose(context.token, Diagnostics::expectedPreprocessorDirectiveName);
            skipToEndOfLine();
            return;
        }

        // Consume the directive name token.
        advanceToken();

        if (context.GetName() != "line")
        {
            GetSink()->Diagnose(context.token, Diagnostics::unknownPreprocessorDirective, context.GetName());
            skipToEndOfLine();
            return;
        }

        handleLineDirective(context);

        // We expect the directive callback to consume the entire line, so if
        // it hasn't that is a parse error.
        expectEndOfDirective(context);
    }

    void LinePreprocessorImpl::handleLineDirective(DirectiveContext& directiveContext)
    {
        uint32_t line = 0;

        switch (peekTokenType())
        {
            case Token::Type::IntegerLiteral:
                line = tokenToUInt(advanceToken(), 10);
                break;

            // `#line` and `#line default` directives are not supported
            case Token::Type::EndOfFile:
            case Token::Type::NewLine:
                expect(directiveContext, Token::Type::IntegerLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                return;

            // else, fall through to:
            default:
                expect(directiveContext, Token::Type::IntegerLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                return;
        }

        PathInfo pathInfo;
        switch (peekTokenType())
        {
            case Token::Type::EndOfFile:
            case Token::Type::NewLine:
                pathInfo = directiveContext.token.sourceLocation.GetSourceView()->GetPathInfo();
                break;

            case Token::Type::StringLiteral:
                pathInfo = PathInfo::makePath(getStringLiteralTokenValue(advanceToken()));
                break;

            default:
                expect(directiveContext, Token::Type::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                return;
        }

        // Do all checking related to the end of this directive before we push a new stream,
        // just to avoid complications where that check would need to deal with
        // a switch of input stream
        expectEndOfDirective(directiveContext);

        const auto& nextToken = peekToken();
        // Start new source view from end of new line sequence.
        const auto sourceLocation = nextToken.sourceLocation + nextToken.stringSlice.GetLength();

        HumaneSourceLocation humaneLocation(line, 1);

        // Todo trash
        pathInfo = PathInfo::makeSplit(pathInfo.foundPath, pathInfo.uniqueIdentity);
        const auto& sourceView = SourceView::CreateSplited(sourceLocation, humaneLocation, pathInfo);

        // Forced closing of the current current stream and start a new one
        popInputFile(true);
        PushInputFile(std::make_shared<InputFile>(shared_from_this(), sourceView));
    }

    LinePreprocessor::~LinePreprocessor() { }

    LinePreprocessor::LinePreprocessor(const std::shared_ptr<Common::LinearAllocator>& allocator,
                                       const std::shared_ptr<DiagnosticSink>& diagnosticSink)
        : impl_(std::make_unique<LinePreprocessorImpl>(allocator, diagnosticSink))
    {
    }

    void LinePreprocessor::PushInputFile(const std::shared_ptr<SourceFile>& sourceFile)
    {
        const auto sourceView = RR::Rfx::SourceView::Create(sourceFile);
        impl_->PushInputFile(std::make_shared<InputFile>(impl_, sourceView));
    }

    std::vector<Token> LinePreprocessor::ReadAllTokens()
    {
        ASSERT(impl_);
        return impl_->ReadAllTokens();
    }
}