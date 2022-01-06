#include "Preprocessor.hpp"

#include "core/IncludeSystem.hpp"

#include "compiler/DiagnosticCore.hpp"
#include "compiler/Lexer.hpp"

#include <filesystem>
#include <sstream>

namespace RR
{
    namespace Rfx
    {
        struct Preprocessor::DirectiveContext
        {
            Token token;
            bool parseError = false;
            bool haveDoneEndOfDirectiveChecks = false;
        };

        namespace
        {
            // Get the name of the directive being parsed.
            U8String getDirectiveName(const Preprocessor::DirectiveContext& context)
            {
                return context.token.GetContentString();
            }

            U8String getFileNameTokenValue(const Token& token)
            {
                const UnownedStringSlice& content = token.stringSlice;

                // A file name usually doesn't process escape sequences
                // (this is import on Windows, where `\\` is a valid
                // path separator character).

                // Just trim off the first and last characters to remove the quotes
                // (whether they were `""` or `<>`.
                return U8String(content.Begin() + 1, content.End() - 1);
            }

            U8String getStringLiteralTokenValue(const Token& token)
            {
                ASSERT(token.type == TokenType::StringLiteral || token.type == TokenType::CharLiteral);

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
                        case 'a':
                            utf8::append('\a', result);
                            continue;
                        case 'b':
                            utf8::append('\b', result);
                            continue;
                        case 'f':
                            utf8::append('\f', result);
                            continue;
                        case 'n':
                            utf8::append('\n', result);
                            continue;
                        case 'r':
                            utf8::append('\r', result);
                            continue;
                        case 't':
                            utf8::append('\t', result);
                            continue;
                        case 'v':
                            utf8::append('\v', result);
                            continue;

                        // clang-format off
                        // Octal escape: up to 3 characterws 
                        case '0': case '1': case '2': case '3': case '4': 
                        case '5': case '6': case '7': 
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
                        }
                            continue;

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
                        }
                            continue;

                            // TODO: Unicode escape sequences
                    }
                }
            }
        }

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
            InputStream() = default;

            // Because different implementations of this abstract base class will
            // store differnet amounts of data, we need a virtual descritor to
            // ensure that we can clean up after them.

            /// Clean up an input stream
            virtual ~InputStream() = default;

            // The two fundamental operations that every input stream must support
            // are reading one token from the stream, and "peeking" one token into
            // the stream to see what will be read next.

            /// Read one token from the input stream
            ///
            /// At the end of the stream should return a token with `TokenType::EndOfFile`.
            ///
            virtual Token ReadToken() = 0;

            /// Peek at the next token in the input stream
            ///
            /// This function should return whatever `readToken()` will return next.
            ///
            /// At the end of the stream should return a token with `TokenType::EndOfFile`.
            ///
            virtual Token PeekToken() = 0;

            // Based on `peekToken()` we can define a few more utility functions
            // for cases where we only care about certain details of the input.

            /// Peek the type of the next token in the input stream.
            TokenType PeekTokenType() { return PeekToken().type; }

            /// Peek the location of the next token in the input stream.
            SourceLocation PeekLoc() { return PeekToken().sourceLocation; }

            /// Get the diagnostic sink to use for messages related to this stream
            // DiagnosticSink* getSink();

            std::shared_ptr<InputStream> GetParent() { return parent_; }

            void SetParent(const std::shared_ptr<InputStream>& parent) { parent_ = parent; }

            // MacroInvocation* getFirstBusyMacroInvocation() { return m_firstBusyMacroInvocation; }

        protected:
            /// The preprocessor that this input stream is being used by
            // Preprocessor* m_preprocessor = nullptr;

            /// Parent stream in the stack of secondary input streams
            std::shared_ptr<InputStream> parent_;

            /// Macro expansions that should be considered "busy" during expansion of this stream
            // MacroInvocation* m_firstBusyMacroInvocation = nullptr;
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

        /// An input stream that reads tokens directly using the Slang `Lexer`
        struct LexerInputStream : InputStream
        {
            LexerInputStream() = delete;

            LexerInputStream(
                const std::shared_ptr<SourceView>& sourceView, const std::shared_ptr<DiagnosticSink>& diagnosticSink)
                : lexer_(new Lexer(sourceView, diagnosticSink))
            {
                lookaheadToken_ = readTokenImpl();
            }

            Lexer& GetLexer() { return *lexer_; }

            // A common thread to many of the input stream implementations is to
            // use a single token of lookahead in order to suppor the `peekToken()`
            // operation with both simplicity and efficiency.

            Token ReadToken() override
            {
                auto result = lookaheadToken_;
                lookaheadToken_ = readTokenImpl();
                return result;
            }

            Token PeekToken() override
            {
                return lookaheadToken_;
            }

        private:
            /// Read a token from the lexer, bypassing lookahead
            Token readTokenImpl()
            {
                for (;;)
                {
                    Token token = lexer_->GetNextToken();
                    switch (token.type)
                    {
                        default:
                            return token;

                        case TokenType::WhiteSpace:
                        case TokenType::BlockComment:
                        case TokenType::LineComment:
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

        Token Preprocessor::dummyToken;

        Preprocessor::~Preprocessor()
        {
        }

        Preprocessor::Preprocessor(const std::shared_ptr<SourceFile>& sourceFile,
                                   const std::shared_ptr<DiagnosticSink>& diagnosticSink,
                                   const std::shared_ptr<IncludeSystem>& includeSystem)
            : sink_(diagnosticSink),
              includeSystem_(includeSystem),
              sourceFile_(sourceFile)
        {
            ASSERT(sourceFile)
            ASSERT(diagnosticSink)
            ASSERT(includeSystem)

            auto sourceView = SourceView::Create(sourceFile_);

            endOfFileToken_.type = TokenType::EndOfFile;

            currentInputStream_ = std::make_shared<LexerInputStream>(sourceView, diagnosticSink);
            // lexer_ = std::make_unique<Lexer>(sourceView, diagnosticSink);
        }

        const std::unordered_map<U8String, Preprocessor::HandleDirectiveFunc> Preprocessor::handleDirectiveFuncMap = {
            //  { "if", nullptr },
            //  { "ifdef", nullptr },
            //  { "ifndef", nullptr },
            //   { "else", nullptr },
            //  { "elif", nullptr },
            //  { "endif", nullptr },
            { "include", &Preprocessor::handleIncludeDirective },
            { "define", &Preprocessor::handleDefineDirective },
            //  { "undef", nullptr },
            //  { "warning", nullptr },
            //  { "error", nullptr },
            { "line", &Preprocessor::handleLineDirective },
            //   { "pragma", nullptr }
        };

        // Look up the directive with the given name.
        Preprocessor::HandleDirectiveFunc Preprocessor::findHandleDirectiveFunc(const U8String& name)
        {
            auto search = handleDirectiveFuncMap.find(name);

            if (search == handleDirectiveFuncMap.end())
                return &Preprocessor::handleInvalidDirective;

            return search->second;
        }

        int32_t Preprocessor::tokenToInt(const Token& token, int radix)
        {
            ASSERT(token.type == TokenType::IntegerLiteral)

            errno = 0;

            auto end = const_cast<U8Char*>(token.stringSlice.End());
            const auto result = std::strtol(token.stringSlice.Begin(), &end, radix);

            if (errno == ERANGE)
                sink_->Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "int32_t");

            if (end == token.stringSlice.End())
                return result;

            sink_->Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), radix);
            return 0;
        }

        uint32_t Preprocessor::tokenToUInt(const Token& token, int radix)
        {
            ASSERT(token.type == TokenType::IntegerLiteral)

            errno = 0;

            auto end = const_cast<U8Char*>(token.stringSlice.End());
            const auto result = std::strtoul(token.stringSlice.Begin(), &end, radix);

            if (errno == ERANGE)
                sink_->Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "uint32_t");

            if (end == token.stringSlice.End())
                return result;

            sink_->Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), radix);
            return 0;
        }

        void Preprocessor::pushInputStream(const std::shared_ptr<InputStream>& inputStream)
        {
            inputStream->SetParent(currentInputStream_);
            currentInputStream_ = inputStream;
        }

        void Preprocessor::popInputStream()
        {
            const auto& inputStream = currentInputStream_;
            ASSERT(inputStream)

            // We expect the file to be at its end, so that the
            // next token read would be an end-of-file token.
            Token eofToken = inputStream->PeekToken(); //TODO PeekRaw
            ASSERT(eofToken.type == TokenType::EndOfFile);

            // If there are any open preprocessor conditionals in the file, then
            // we need to diagnose them as an error, because they were not closed
            // at the end of the file. TODO
            /* for (auto conditional = inputFile->getInnerMostConditional(); conditional; conditional = conditional->parent)
                {
                    GetSink(this)->diagnose(eofToken, Diagnostics::endOfFileInPreprocessorConditional);
                    GetSink(this)->diagnose(conditional->ifToken, Diagnostics::seeDirective, conditional->ifToken.getContent());
                */

            // We will update the current file to the parent of whatever
            // the `inputFile` was (usually the file that `#include`d it).
            //
            auto parentFile = inputStream->GetParent();
            currentInputStream_ = parentFile;

            // As a subtle special case, if this is the *last* file to be popped,
            // then we will update the canonical EOF token used by the preprocessor
            // to be the EOF token for `inputFile`, so that the source location
            // information returned will be accurate.
            if (!parentFile)
                endOfFileToken_ = eofToken;
        }

        std::vector<Token> Preprocessor::ReadAllTokens()
        {
            std::vector<Token> tokens;

            for (;;)
            {
                const auto& token = ReadToken();

                switch (token.type)
                {
                    default:
                        tokens.push_back(token);
                        break;

                    case TokenType::EndOfFile:
                        // Note: we include the EOF token in the list,
                        // since that is expected by the `TokenList` type.
                        tokens.push_back(token);
                        return tokens;

                    case TokenType::WhiteSpace:
                    case TokenType::NewLine:
                    case TokenType::LineComment:
                    case TokenType::BlockComment:
                    case TokenType::Invalid:
                        break;
                }
            }
        }

        Token Preprocessor::peekToken()
        {
            // TODO
            return currentInputStream_->PeekToken();
        }

        Token Preprocessor::peekRawToken()
        {
            return currentInputStream_->PeekToken();
        }

        Token Preprocessor::advanceToken()
        {
            if (isEndOfLine())
                return peekRawToken();
            return currentInputStream_->ReadToken();
        }

        Token Preprocessor::advanceRawToken()
        {
            // TODO read rawtoken
            return currentInputStream_->ReadToken();
        }

        void Preprocessor::skipToEndOfLine()
        {
            while (!isEndOfLine())
                advanceRawToken();
        }

        bool Preprocessor::isEndOfLine()
        {
            switch (currentInputStream_->PeekTokenType())
            {
                case TokenType::EndOfFile:
                case TokenType::NewLine:
                    return true;

                default:
                    return false;
            }
        }

        Token Preprocessor::ReadToken()
        {
            for (;;)
            {
                if (!currentInputStream_)
                    return endOfFileToken_;
                /*
                    auto expansionStream = InputSource->getExpansionStream();

                    // Look at the next raw token in the input.
                    Token token = expansionStream->peekRawToken();     */
                Token token = currentInputStream_->PeekToken(); // PeekRawToken
                if (token.type == TokenType::EndOfFile)
                {
                    popInputStream();
                    continue;
                }

                // If we have a directive (`#` at start of line) then handle it
                if (token.type == TokenType::Directive)
                {
                    handleDirective();
                    continue;
                }
                /*
                 
                    if ((token.type == TokenType::Pound) && (token.flags & TokenFlag::AtStartOfLine))
                    {
                        



                        // Parse and handle the directive
                        HandleDirective(&directiveContext);
                        continue;
                    }

                    // otherwise, if we are currently in a skipping mode, then skip tokens
                    if (InputSource->isSkipping())
                    {
                        expansionStream->readRawToken();
                        continue;
                    }*/

                token = currentInputStream_->PeekToken();
                if (token.type == TokenType::EndOfFile)
                {
                    popInputStream();
                    continue;
                }

                currentInputStream_->ReadToken();
                return token;
            }
        }

        bool Preprocessor::expect(DirectiveContext& context, TokenType expected, DiagnosticInfo const& diagnostic, Token& outToken)
        {
            if (peekTokenType() != expected)
            {
                // Only report the first parse error within a directive
                if (!context.parseError)
                    sink_->Diagnose(context.token, diagnostic, expected, getDirectiveName(context));

                context.parseError = true;
                return false;
            }

            outToken = advanceToken();
            return true;
        }

        bool Preprocessor::expectRaw(DirectiveContext& context, TokenType expected, DiagnosticInfo const& diagnostic, Token& outToken)
        {
            if (peekRawTokenType() != expected)
            {
                // Only report the first parse error within a directive
                if (!context.parseError)
                    sink_->Diagnose(context.token, diagnostic, expected, getDirectiveName(context));

                context.parseError = true;
                return false;
            }

            outToken = advanceRawToken();
            return true;
        }

        void Preprocessor::handleDirective()
        {
            ASSERT(peekRawTokenType() == TokenType::Directive)

            // Skip the `#`
            currentInputStream_->ReadToken(); // TODO ReadRawToken

            // Create a context for parsing the directive
            DirectiveContext context;

            // Try to read the directive name.
            context.token = peekRawToken();

            TokenType directiveTokenType = context.token.type;

            // An empty directive is allowed, and ignored.
            switch (directiveTokenType)
            {
                case TokenType::EndOfFile:
                case TokenType::NewLine:
                    return;

                default:
                    break;
            }

            // Otherwise the directive name had better be an identifier
            if (directiveTokenType != TokenType::Identifier)
            {
                sink_->Diagnose(context.token, Diagnostics::expectedPreprocessorDirectiveName);
                skipToEndOfLine();
                return;
            }

            // Consume the directive
            advanceRawToken();

            // Look up the handler for the directive.
            const auto directiveHandler = findHandleDirectiveFunc(getDirectiveName(context));

            // Call the directive-specific handler
            (this->*directiveHandler)(context);
        }

        void Preprocessor::handleInvalidDirective(DirectiveContext& directiveContext)
        {
            sink_->Diagnose(directiveContext.token, Diagnostics::unknownPreprocessorDirective, getDirectiveName(directiveContext));
            skipToEndOfLine();
        }

        void Preprocessor::handleDefineDirective(DirectiveContext&)
        {

            // Token nameToken;
            // if (!expectRaw(directiveContext, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective))
            //    return;
            // Name* name = nameToken.getName();
        }

        void Preprocessor::handleIncludeDirective(DirectiveContext& directiveContext)
        {
            Token pathToken;
            if (!expect(directiveContext, TokenType::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective, pathToken))
                return;

            U8String path = getFileNameTokenValue(pathToken);

            const auto& sourceView = directiveContext.token.sourceLocation.GetSourceView();
            const auto& includedFromPathInfo = sourceView->GetSourceFile();

            (void)includedFromPathInfo;

            /* Find the path relative to the foundPath */
            PathInfo filePathInfo;
            if (RFX_FAILED(includeSystem_->FindFile(path, includedFromPathInfo->GetPathInfo().foundPath, filePathInfo)))
            {
                sink_->Diagnose(peekRawToken(), Diagnostics::includeFailed, path);
                return;
            }

            // We must have a uniqueIdentity to be compare
            if (!filePathInfo.hasUniqueIdentity())
            {
                sink_->Diagnose(peekRawToken(), Diagnostics::noUniqueIdentity, path);
                return;
            }

            // Do all checking related to the end of this directive before we push a new stream,
            // just to avoid complications where that check would need to deal with
            // a switch of input stream
            expectEndOfDirective(directiveContext);

            // Check whether we've previously included this file and seen a `#pragma once` directive
            if (pragmaOnceUniqueIdentities_.find(filePathInfo.uniqueIdentity) != pragmaOnceUniqueIdentities_.end())
                return;

            // Simplify the path
            filePathInfo.foundPath = std::filesystem::path(filePathInfo.foundPath).lexically_normal().u8string();

            std::shared_ptr<SourceFile> includedFile;

            if (RFX_FAILED(includeSystem_->LoadFile(filePathInfo, includedFile)))
            {
                sink_->Diagnose(peekRawToken(), Diagnostics::includeFailed, path);
                return;
            }

            const auto& includedView = SourceView::CreateIncluded(includedFile, sourceView, directiveContext.token.humaneSourceLocation);
            const auto& inputStream = std::make_shared<LexerInputStream>(includedView, sink_);

            pushInputStream(inputStream);
        }

        void Preprocessor::handleLineDirective(DirectiveContext& directiveContext)
        {
            uint32_t line = 0;

            switch (peekTokenType())
            {
                case TokenType::IntegerLiteral:
                    line = tokenToUInt(advanceToken(), 10);
                    break;

                // `#line` and `#line default` directives are not supported
                case TokenType::EndOfFile:
                case TokenType::NewLine:
                    expect(directiveContext, TokenType::IntegerLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                    return;

                /* else, fall through to: */
                default:
                    expect(directiveContext, TokenType::IntegerLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                    return;
            }

            PathInfo pathInfo;
            switch (peekTokenType())
            {
                case TokenType::EndOfFile:
                case TokenType::NewLine:
                    pathInfo = directiveContext.token.sourceLocation.GetSourceView()->GetPathInfo();
                    break;

                case TokenType::StringLiteral:
                    pathInfo = PathInfo::makePath(getStringLiteralTokenValue(advanceToken()));
                    break;

                default:
                    expect(directiveContext, TokenType::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective);
                    return;
            }

            // Do all checking related to the end of this directive before we push a new stream,
            // just to avoid complications where that check would need to deal with
            // a switch of input stream
            expectEndOfDirective(directiveContext);

            const auto& nextToken = currentInputStream_->PeekToken();
            // Start new source view from end of new line sequence.
            const auto sourceLocation = nextToken.sourceLocation + nextToken.stringSlice.GetLength();

            HumaneSourceLocation humaneLocation(line, 1);
            const auto& sourceView = SourceView::CreateSplited(sourceLocation, humaneLocation, pathInfo);
            const auto& inputStream = std::make_shared<LexerInputStream>(sourceView, sink_);

            pushInputStream(inputStream);
        }

        void Preprocessor::expectEndOfDirective(DirectiveContext& context)
        {
            if (context.haveDoneEndOfDirectiveChecks)
                return;

            context.haveDoneEndOfDirectiveChecks = true;

            if (!isEndOfLine())
            {
                // If we already saw a previous parse error, then don't
                // emit another one for the same directive.
                if (!context.parseError)
                {
                    sink_->Diagnose(context.token, Diagnostics::unexpectedTokensAfterDirective, getDirectiveName(context));
                }
                skipToEndOfLine();
            }
        }
    }
}