#include "Preprocessor.hpp"

#include "parse_tools/core/CompileContext.hpp"
#include "parse_tools/DiagnosticCore.hpp"
#include "parse_tools/Lexer.hpp"

#include "parse_tools/core/IncludeSystem.hpp"
#include "parse_tools/core/StringEscapeUtil.hpp"

#include "common/LinearAllocator.hpp"
#include "common/Result.hpp"

#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace RR
{
    namespace ParseTools
    {
        namespace
        {
            std::string getFileNameTokenValue(const Token& token)
            {
                const UnownedStringSlice& content = token.stringSlice;

                // A file name usually doesn't process escape sequences
                // (this is import on Windows, where `\\` is a valid
                // path separator character).
                return std::string(content.begin(), content.end());
            }

            std::string getStringLiteralTokenValue(const Token& token)
            {
                ASSERT(token.type == Token::Type::StringLiteral || token.type == Token::Type::CharLiteral);

                const UnownedStringSlice content = token.stringSlice;

                auto cursor = utf8::iterator<const char*>(&*content.begin(), &*content.begin(), &*content.end());
                auto end = utf8::iterator<const char*>(&*content.end(), &*content.begin(), &*content.end());

                auto quote = *cursor++;
                ASSERT(quote == '\'' || quote == '"');

                std::string result;
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

            // Determine the precedence level of an infix operator
            // for use in parsing preprocessor conditionals.
            int32_t GetInfixOpPrecedence(Token const& opToken)
            {
                // If token is on another line, it is not part of the
                // expression
                if (Common::IsSet(opToken.flags, Token::Flags::AtStartOfLine))
                    return -1;

                // otherwise we look at the token type to figure
                // out what precedence it should be parse with
                switch (opToken.type)
                {
                    default:
                        // tokens that aren't infix operators should
                        // cause us to stop parsing an expression
                        return -1;

                        // clang-format off
                    case Token::Type::OpMul:     return 10;
                    case Token::Type::OpDiv:     return 10;
                    case Token::Type::OpMod:     return 10;

                    case Token::Type::OpAdd:     return 9;
                    case Token::Type::OpSub:     return 9;

                    case Token::Type::OpLsh:     return 8;
                    case Token::Type::OpRsh:     return 8;

                    case Token::Type::OpLess:    return 7;
                    case Token::Type::OpGreater: return 7;
                    case Token::Type::OpLeq:     return 7;
                    case Token::Type::OpGeq:     return 7;

                    case Token::Type::OpEql:     return 6;
                    case Token::Type::OpNeq:     return 6;

                    case Token::Type::OpBitAnd:  return 5;
                    case Token::Type::OpBitOr:   return 4;
                    case Token::Type::OpBitXor:  return 3;
                    case Token::Type::OpAnd:     return 2;
                    case Token::Type::OpOr:      return 1; // clang-format on
                }
            };
        }

        struct InputFile;
        struct MacroInvocation;
        struct MacroDefinition;
        using PreprocessorExpressionValue = int32_t;

        //
        // Utility Types
        //

        /// A preprocessor conditional construct that is currently active.
        ///
        /// This type handles preprocessor conditional structures like
        /// `#if` / `#elif` / `#endif`. A single top-level input file
        /// will have some number of "active" conditionals at one time,
        /// based on the nesting depth of those conditional structures.
        ///
        /// Each conditional may be in a distinct state, which decides
        /// whether tokens should be skipped or not.
        struct Conditional
        {
            /// A state that a preprocessor conditional can be in.
            ///
            /// The state of a conditional depends both on what directives
            /// have been encountered so far (e.g., just an `#if`, or an
            /// `#if` and then an `#else`), as well as what the value
            /// of any conditions related to those directives have been.
            enum class State
            {
                /// Indicates that this conditional construct has not yet encountered a branch with a `true` condition.
                /// The preprocessor should skip tokens, but should keep scanning and evaluating branch conditions.
                Before,

                /// Indicates that this conditional construct is nested inside the branch with a `true` condition
                /// The preprocessor should not skip tokens, and should not bother evaluating subsequent branch conditions.
                During,

                /// Indicates that this conditional has laready seen the branch with a `true` condition
                /// The preprocessor should skip tokens, and should not bother evaluating subsequent branch conditions.
                After,
            };

            /// The next outer conditional in the current input file, or nullptr if this is the outer-most conditional.
            std::shared_ptr<Conditional> parent;

            /// The token that started the conditional (e.g., an `#if` or `#ifdef`)
            Token ifToken;

            /// The `#else` directive token, if one has been seen (otherwise has `TokenType::Unknown`)
            Token elseToken;

            /// The state of the conditional
            State state;
        };

        struct DirectiveContext : Common::NonCopyable
        {
        public:
            // Get the name of the directive being parsed.
            inline std::string GetName() const { return token.GetContentString(); }

        public:
            Token token;
            std::shared_ptr<InputFile> inputFile;
            bool parseError = false;
            bool haveDoneEndOfDirectiveChecks = false;
        };

        class PreprocessorImpl final : Common::NonCopyable
        {
        public:
            typedef void (PreprocessorImpl::*HandleDirectiveFunc)(DirectiveContext& context);
            typedef void (PreprocessorImpl::*HandlePragmaDirectiveFunc)(DirectiveContext& context, const Token& subDirectiveToken);

            struct Directive
            {
                enum class Flags : uint32_t
                {
                    None = 0,

                    // Should this directive be handled even when skipping disbaled code?
                    ProcessWhenSkipping = 1 << 0,

                    /// Allow the handler for this directive to advance past the
                    /// directive token itself, so that it can control lexer behavior
                    /// more closely.
                    DontConsumeDirectiveAutomatically = 1 << 1,
                };

                Flags flags;
                HandleDirectiveFunc function;
            };

            struct PragmaDirective
            {
                HandlePragmaDirectiveFunc function;
            };

        public:
            PreprocessorImpl(const std::shared_ptr<IncludeSystem>& includeSystem,
                             const std::shared_ptr<SourceManager>& sourceManager,
                             const std::shared_ptr<CompileContext>& context);

            DiagnosticSink& GetSink() const { return context_->sink; }
            SourceManager& GetSourceManager() const { return *sourceManager_; }
            IncludeSystem GetIncludeSystem() const { return *includeSystem_; }
            std::shared_ptr<CompileContext> GetContext() const { return context_; }
            auto& GetAllocator() const { return context_->allocator; }

            std::vector<Token> ReadAllTokens();

            void DefineMacro(const std::string& macro);
            void DefineMacro(const std::string& key, const std::string& value);

            // Find the currently-defined macro of the given name, or return nullptr
            std::shared_ptr<MacroDefinition> LookupMacro(const std::string& name) const;

            // Push a new input file onto the input stack of the preprocessor
            void PushInputFile(const std::shared_ptr<InputFile>& inputFile);

        private:
            int32_t tokenToInt(const Token& token, int radix);
            uint32_t tokenToUInt(const Token& str, int radix);

            Token readToken();

            // Pop the inner-most input file from the stack of input files
            // Force - skips the eof check
            void popInputFile(bool force = false);

            inline Token peekRawToken();
            inline Token::Type peekRawTokenType() { return peekRawToken().type; }

            inline Token peekToken();
            inline Token::Type peekTokenType() { return peekToken().type; }

            // Read one token, with macro-expansion, without going past the end of the line.
            Token advanceToken();

            // Read one raw token, without going past the end of the line.
            Token advanceRawToken();

            // Skip to the end of the line (useful for recovering from errors in a directive)
            void skipToEndOfLine();

            // Determine if we have read everything on the directive's line.
            bool isEndOfLine();

            bool expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic);
            bool expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken);

            bool expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic);
            bool expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken);

            std::string readDirectiveMessage();

            void beginConditional(const DirectiveContext& context, bool enable);

            void handleDirective();
            void handleInvalidDirective(DirectiveContext& directiveContext);

            void handleIfDirective(DirectiveContext& directiveContext);
            void handleIfdefDirective(DirectiveContext& directiveContext);
            void handleIfndefDirective(DirectiveContext& directiveContext);
            void handleElseDirective(DirectiveContext& directiveContext);
            void handleElifDirective(DirectiveContext& directiveContext);
            void handleEndIfDirective(DirectiveContext& directiveContext);

            void handleDefineDirective(DirectiveContext& directiveContext);
            void handleUndefDirective(DirectiveContext& directiveContext);
            void handleWarningDirective(DirectiveContext& directiveContext);
            void handleErrorDirective(DirectiveContext& directiveContext);
            void handleIncludeDirective(DirectiveContext& directiveContext);
            void handleLineDirective(DirectiveContext& directiveContext);
            void handlePragmaDirective(DirectiveContext& directiveContext);

            void handleUnknownPragmaDirective(DirectiveContext& directiveContext, const Token& subDirectiveToken);
            void handlePragmaOnceDirective(DirectiveContext& directiveContext, const Token& subDirectiveToken);

            PreprocessorExpressionValue evaluateInfixOp(
                const DirectiveContext& context,
                const Token& opToken,
                PreprocessorExpressionValue left,
                PreprocessorExpressionValue right);

            PreprocessorExpressionValue skipOrParseAndEvaluateExpression(DirectiveContext& directiveContext);
            PreprocessorExpressionValue parseAndEvaluateExpression(DirectiveContext& directiveContext);
            PreprocessorExpressionValue parseAndEvaluateUnaryExpression(DirectiveContext& directiveContext);
            PreprocessorExpressionValue parseAndEvaluateInfixExpressionWithPrecedence(
                DirectiveContext& context,
                PreprocessorExpressionValue left,
                int32_t precedence);

            // Helper routine to check that we find the end of a directive where we expect it.
            // Most directives do not need to call this directly, since we have
            // a catch-all case in the main `handleDirective()` function.
            // `#include` and `#line` case will call it directly to avoid complications
            // when it switches the input stream.
            void expectEndOfDirective(DirectiveContext& context);

            void parseMacroOps(const std::shared_ptr<MacroDefinition>& macro,
                               const std::unordered_map<std::string, uint32_t>& mapParamNameToIndex);

        private:
            std::shared_ptr<IncludeSystem> includeSystem_;
            std::shared_ptr<SourceManager> sourceManager_;
            std::shared_ptr<CompileContext> context_;

            /// A stack of "active" input files
            std::shared_ptr<InputFile> currentInputFile_;

            /// The unique identities of any paths that have issued `#pragma once` directives to
            /// stop them from being included again.
            std::unordered_set<std::string> pragmaOnceUniqueIdentities_;

            /// A pre-allocated token that can be returned to represent end-of-input situations.
            Token endOfFileToken_;

            /// Macros defined in this environment
            std::unordered_map<std::string, std::shared_ptr<MacroDefinition>> macrosDefinitions_;

        private:
            // Look up the directive with the given name.
            static Directive findDirective(const std::string& name)
            {
                auto search = directiveMap.find(name);

                if (search == directiveMap.end())
                    return {Directive::Flags::None, &PreprocessorImpl::handleInvalidDirective};

                return search->second;
            }

            // Look up the `#pragma` directive with the given name.
            static PragmaDirective findPragmaDirective(const std::string& name)
            {
                auto search = paragmaDirectiveMap.find(name);

                if (search == paragmaDirectiveMap.end())
                    return {&PreprocessorImpl::handleUnknownPragmaDirective};

                return search->second;
            }

        private:
            static const std::unordered_map<std::string, Directive> directiveMap;
            static const std::unordered_map<std::string, PragmaDirective> paragmaDirectiveMap;
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
        struct InputStream : Common::NonCopyable
        {
        public:
            InputStream(const PreprocessorImpl& preprocessor)
                : preprocessor_(&preprocessor) { }

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

            SourceManager& GetSourceManager() const { return preprocessor_->GetSourceManager(); }
            DiagnosticSink& GetSink() const { return preprocessor_->GetSink(); }

            std::shared_ptr<InputStream> GetParent() const { return parent_; }
            void SetParent(const std::shared_ptr<InputStream>& parent) { parent_ = parent; }

            MacroInvocation* GetFirstBusyMacroInvocation() const { return firstBusyMacroInvocation_; }

        protected:
            /// The preprocessor that this input stream is being used by
            const PreprocessorImpl* preprocessor_;

            /// Parent stream in the stack of secondary input streams
            std::shared_ptr<InputStream> parent_;

            /// Macro expansions that should be considered "busy" during expansion of this stream
            MacroInvocation* firstBusyMacroInvocation_ = nullptr;
        };

        // During macro expansion, or the substitution of parameters into a macro body
        // we end up needing to track multiple active input streams, and this is most
        // easily done by having a distinct type to represent a stack of input streams.

        /// A stack of input streams, that will always read the next available token from the top-most stream
        /// An input stream stack assumes ownership of all streams pushed onto it, and will clean them
        /// up when they are no longer active or when the stack gets destructed.
        struct InputStreamStack
        {
            InputStreamStack() { }

            /// Clean up after an input stream stack
            ~InputStreamStack() { PopAll(); }

            /// Push an input stream onto the stack
            void Push(const std::shared_ptr<InputStream>& stream)
            {
                stream->SetParent(top_);
                top_ = stream;
            }

            /// Pop all input streams on the stack
            void PopAll() { top_ = nullptr; }

            /// Read a token from the top-most input stream with input
            ///
            /// If there is no input remaining, will return the EOF token
            /// of the bottom-most stream.
            ///
            /// At least one input stream must have been `push()`ed before
            /// it is valid to call this operation.
            Token ReadToken()
            {
                ASSERT(top_);
                for (;;)
                {
                    // We always try to read from the top-most stream, and if
                    // it is not at its end, then we return its next token.
                    auto token = top_->ReadToken();
                    if (token.type != Token::Type::EndOfFile)
                        return token;

                    // If the top stream has run out of input we try to
                    // switch to its parent, if any.
                    auto parent = top_->GetParent();
                    if (parent)
                    {
                        // This stack has taken ownership of the streams,
                        // and must therefore delete the top stream before
                        // popping it.
                        top_ = parent;
                        continue;
                    }

                    // If the top stream did *not* have a parent (meaning
                    // it was also the bottom stream), then we don't try
                    // to pop it and instead return its EOF token as-is.
                    return token;
                }
            }

            /// Peek a token from the top-most input stream with input
            ///
            /// If there is no input remaining, will return the EOF token
            /// of the bottom-most stream.
            ///
            /// At least one input stream must have been `push()`ed before
            /// it is valid to call this operation.
            Token PeekToken()
            {
                // The logic here mirrors `readToken()`, but we do not
                // modify the `top_` value or delete streams when they
                // are at their end, so that we don't disrupt any state
                // that might depend on which streams are present on
                // the stack.
                //
                // Note: One might ask why we cannot just pop input
                // streams that are at their end immediately. The basic
                // reason has to do with determining what macros were
                // "busy" when considering expanding a new one.
                // Consider:
                //
                //      #define BAD A B C BAD
                //
                //      BAD X Y Z
                //
                // When expanding the invocation of `BAD`, we will eventually
                // reach a point where the `BAD` in the expansion has been read
                // and we are considering whether to consider it as a macro
                // invocation.
                //
                // In this case it is clear that the Right Answer is that the
                // original invocation of `BAD` is still active, and thus
                // the macro is busy. To ensure that behavior, we want to
                // be able to detect that the stream representing the
                // expansion of `BAD` is still active even after we read
                // the `BAD` token.
                //
                // TODO: Consider whether we can streamline the implementaiton
                // an remove this wrinkle.
                auto top = top_;
                for (;;)
                {
                    ASSERT(top);
                    auto token = top->PeekToken();
                    if (token.type != Token::Type::EndOfFile)
                        return token;

                    auto parent = top->GetParent();
                    if (parent)
                    {
                        top = parent;
                        continue;
                    }

                    return token;
                }
            }

            /// Return type of the token that `peekToken()` will return
            Token::Type PeekTokenType() { return PeekToken().type; }

            /// Skip over all whitespace tokens in the input stream(s) to arrive at the next non-whitespace token
            void SkipAllWhitespace()
            {
                for (;;)
                {
                    switch (PeekTokenType())
                    {
                        default:
                            return;

                        // Note: We expect `NewLine` to be the only case of whitespace we
                        // encounter right now, because all the other cases will have been
                        // filtered out by the `LexerInputStream`.
                        case Token::Type::NewLine:
                        case Token::Type::WhiteSpace:
                        case Token::Type::BlockComment:
                        case Token::Type::LineComment:
                            ReadToken();
                            break;
                    }
                }
            }

            /// Get the top stream of the input stack
            std::shared_ptr<InputStream> GetTopStream() { return top_; }

            /// Get the input stream that the next token would come from
            /// If the input stack is at its end, this will just be the top-most stream.
            std::shared_ptr<InputStream> GetNextStream()
            {
                ASSERT(top_);
                auto top = top_;
                for (;;)
                {
                    auto tokenType = top->PeekTokenType();
                    if (tokenType != Token::Type::EndOfFile)
                        return top;

                    auto parent = top->GetParent();
                    if (parent)
                    {
                        top = parent;
                        continue;
                    }

                    return top;
                }
            }

        private:
            /// The top of the stack of input streams
            std::shared_ptr<InputStream> top_ = nullptr;
        };

        // The simplest types of input streams are those that simply "play back"
        // a list of tokens that was already captures. These types of streams
        // are primarily used for playing back the tokens inside of a macro body.

        /// An input stream that reads from a list of tokens that had already been tokenized before.
        struct PretokenizedInputStream : InputStream
        {
        public:
            /// Initialize an input stream with list of `tokens`
            PretokenizedInputStream(const PreprocessorImpl& preprocessor, const TokenReader& tokens)
                : InputStream(preprocessor), tokenReader_(tokens) { }

            // A pretokenized stream implements the key read/peek operations
            // by delegating to the underlying token reader.
            virtual Token ReadToken() override { return tokenReader_.AdvanceToken(); }
            virtual Token PeekToken() override { return tokenReader_.PeekToken(); }

        protected:
            PretokenizedInputStream(const PreprocessorImpl& preprocessor)
                : InputStream(preprocessor) {};

            /// Reader for pre-tokenized input
            TokenReader tokenReader_;
        };

        // While macro bodies are the main use case for pre-tokenized input strams,
        // we also use them for a few one-off cases where the preprocessor needs to
        // construct one or more tokens on the fly (e.g., when stringizing or pasting
        // tokens). These streams differ in that they own the storage for the tokens
        // they will play back, because they are effectively "one-shot."

        /// A pre-tokenized input stream that will only be used once, and which therefore owns the memory for its tokens.
        struct SingleUseInputStream : PretokenizedInputStream
        {
            SingleUseInputStream(const PreprocessorImpl& preprocessor, const TokenList& lexedTokens)
                : PretokenizedInputStream(preprocessor), lexedTokens_(lexedTokens)
            {
                tokenReader_ = TokenReader(lexedTokens_);
            }

            /// A list of raw tokens that will provide input
            TokenList lexedTokens_;
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

            LexerInputStream(const PreprocessorImpl& preprocessorImpl, const std::shared_ptr<SourceView>& sourceView)
                : InputStream(preprocessorImpl)
            {
                lexer_ = std::make_unique<Lexer>(sourceView, preprocessorImpl.GetContext());
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
                        default: return token;
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

        // The remaining input stream cases deal with macro expansion, so it is
        // probalby a good idea to discuss how macros are represented by the
        // preprocessor as a first step.
        //
        // Note that there is an important distinction between a macro *definition*
        // and a macro *invocation*, similar to how we distinguish a function definition
        // from a call to that function.

        /// A definition of a macro
        struct MacroDefinition
        {
        public:
            /// The "flavor" / type / kind of a macro definition
            enum class Flavor
            {
                /// A function-like macro (e.g., `#define INC(x) (x)++`)
                FunctionLike,

                /// An user-defiend object-like macro (e.g., `#define N 100`)
                ObjectLike,

                /// An object-like macro that is built in to the copmiler (e.g., `__LINE__`)
                BuiltinObjectLike,
            };

            // The body of a macro definition is input as a stream of tokens, but
            // when "playing back" a macro it is helpful to process those tokens
            // into a form where a lot of the semantic questions have been answered.
            //
            // We will chop up the tokens that macro up a macro definition/body into
            // distinct *ops* where each op has an *opcode* that defines how that
            // token or range of tokens behaves.

            /// Opcode for an `Op` in a macro definition
            enum class Opcode
            {
                /// A raw span of tokens from the macro body (no subsitution needed)
                ///
                /// The `index0` and `index1` fields form a begin/end pair of tokens
                RawSpan,

                /// A parameter of the macro, which should have expansion applied to it
                ///
                /// The `index0` opcode is the index of the token that named the parameter
                /// The `index1` field is the zero-based index of the chosen parameter
                ExpandedParam,

                /// A parameter of the macro, which should *not* have expansion applied to it
                ///
                /// The `index0` opcode is the index of the token that named the parameter
                /// The `index1` field is the zero-based index of the chosen parameter
                UnexpandedParam,

                /// A parameter of the macro, stringized (and not expanded)
                ///
                /// The `index0` opcode is the index of the token that named the parameter
                /// The `index1` field is the zero-based index of the chosen parameter
                StringizedParam,

                /// A paste of the last token of the preceding op and the first token of the next
                ///
                /// The `index0` opcode is the index of the `##` token
                TokenPaste,

                /// builtin expansion behavior for `__LINE__`
                BuiltinLine,

                /// builtin expansion behavior for `__FILE__`
                BuiltinFile,
            };

            /// A single op in the definition of the macro
            struct Op
            {
                /// The opcode that defines how to interpret this op
                Opcode opcode = Opcode::RawSpan;

                /// Two operands, with interpretation depending on the `opcode`
                uint32_t index0 = 0;
                uint32_t index1 = 0;

                // Comments
                Token::Flags flags = Token::Flags::None;
            };

            struct Param
            {
                std::string name;
                SourceLocation sourceLocation;
                bool isVariadic = false;
            };

        public:
            std::string GetName() const { return name; }
            Token GetNameToken() const { return nameToken; }
            bool IsBuiltin() const { return flavor == MacroDefinition::Flavor::BuiltinObjectLike; }

            /// Is this a variadic macro?
            bool IsVariadic() const
            {
                // A macro is variadic if it has a last parameter and
                // that last parameter is a variadic parameter.
                auto paramCount = params.size();
                if (paramCount == 0)
                    return false;

                return params[paramCount - 1].isVariadic;
            }

        public:
            /// The flavor of macro
            MacroDefinition::Flavor flavor;

            /// The name under which the macro was `#define`d
            /// TODO: replace name with uniqueidentifier for better performance
            std::string name;

            /// The name token of macro
            Token nameToken;

            /// The tokens that make up the macro body
            std::vector<Token> tokens;

            /// List ops that describe how this macro expands
            std::vector<Op> ops;

            /// Parameters of the macro, in case of a function-like macro
            std::vector<Param> params;
        };

        // When a macro is invoked, we conceptually want to "play back" the ops
        // that make up the macro's definition. The `MacroInvocation` type logically
        // represents an invocation of a macro and handles the complexities of
        // playing back its definition with things like argument substiution.

        /// An invocation/call of a macro, which can provide tokens of its expansion
        struct MacroInvocation final : InputStream
        {
        public:
            /// Create a new expansion of `macro`
            MacroInvocation(
                const PreprocessorImpl& preprocessor,
                const std::shared_ptr<MacroDefinition>& macro,
                const SourceLocation& macroInvocationLoc,
                const Token& initiatingMacroToken);

            /// Prime the input stream
            ///
            /// This operation *must* be called before the first `readToken()` or `peekToken()`
            void Prime(MacroInvocation* nextBusyMacroInvocation);

            // The `readToken()` and `peekToken()` operations for a macro invocation
            // will be implemented by using one token of lookahead, which makes the
            // operations relatively simple.

            virtual Token ReadToken() override
            {
                Token result = lookaheadToken_;
                lookaheadToken_ = readTokenImpl();
                return result;
            }

            virtual Token PeekToken() override { return lookaheadToken_; }

            /// Is the given `macro` considered "busy" during the given macroinvocation?
            static bool IsBusy(const std::shared_ptr<MacroDefinition>& macro, MacroInvocation* duringMacroInvocation);

            size_t GetArgCount() { return args_.size(); }

        private:
            // Macro invocations are created as part of applying macro expansion
            // to a stream, so the `ExpansionInputStream` type takes responsibility
            // for setting up much of the state of a `MacroInvocation`.
            friend struct ExpansionInputStream;

            /// Actually read a new token (not just using the lookahead)
            Token readTokenImpl();

            /// Initialize the input stream for the current macro op
            void initCurrentOpStream();

            /// Get a reader for the tokens that make up the macro argument at the given `paramIndex`
            TokenReader getArgTokens(uint32_t paramIndex);

            void initPastedSourceViewForTokens(TokenReader& tokenReader, const Token& initiatingToken, TokenList& outTokenList) const;

            /// Push a stream onto `currentOpStreams_` that consists of a single token
            void pushSingleTokenStream(Token::Type tokenType, const SourceLocation& sourceLocation, const HumaneSourceLocation& humaneSourceLocation, std::string const& content);

            /// Push a stream for a source-location builtin (`__FILE__` or `__LINE__`), with content set up by `valueBuilder`
            template <typename F>
            void pushStreamForSourceLocBuiltin(Token::Type tokenType, F const& valueBuilder);

        private:
            /// The macro being expanded
            std::shared_ptr<MacroDefinition> macro_;

            /// A single argument to the macro invocation
            ///
            /// Each argument is represented as a begin/end pair of indices
            /// into the sequence of tokens that make up the macro arguments.
            struct Arg
            {
                uint32_t beginTokenIndex = 0;
                uint32_t endTokenIndex = 0;
            };

            /// Tokens that make up the macro arguments, in case of function-like macro expansion
            std::vector<Token> argTokens_;

            /// Arguments to the macro, in the case of a function-like macro expansion
            std::vector<Arg> args_;

            /// Additional macros that should be considered "busy" during this expansion
            MacroInvocation* nextBusyMacroInvocation_ = nullptr;

            /// Locatin of the macro invocation that led to this expansion
            SourceLocation macroInvocationLoc_;

            /// "iniating" macro token invocation in cases where multiple
            /// nested macro invocations might be in flight.
            Token initiatingMacroToken_;

            /// One token of lookahead
            Token lookaheadToken_;

            // In order to play back a macro definition, we will play back the ops
            // in its body one at a time. Each op may expand to a stream of zero or
            // more tokens, so we need some state to track all of that.

            /// One or more input streams representing the current "op" being expanded
            InputStreamStack currentOpStreams_;

            /// The index into the macro's list of the current operation being played back
            uint32_t macroOpIndex_ = 0;
        };

        // Playing back macro bodies for macro invocations is one part of the expansion process, and the other
        // is scanning through a token stream and identifying macro invocations that need to be expanded.
        // Rather than have one stream type try to handle both parts of the process, we use a distinct type
        // to handle scanning for macro invocations.
        //
        // By using two distinct stream types we are able to handle intriciate details of the C/C++ preprocessor
        // like how the argument tokens to a macro are expanded before they are subsituted into the body, and then
        // are subject to another round of macro expansion *after* substitution.

        /// An input stream that applies macro expansion to another stream
        struct ExpansionInputStream : InputStream
        {
        public:
            /// Construct an input stream that applies macro expansion to `base`
            ExpansionInputStream(const PreprocessorImpl& preprocessor,
                                 const std::shared_ptr<InputStream>& base)
                : InputStream(preprocessor), base_(base)
            {
                inputStreams_.Push(base);
                lookaheadToken_ = readTokenImpl();
            }

            Token ReadToken() override
            {
                // Reading a token from an expansion strema amounts to checking
                // whether the current state of the input stream marks the start
                // of a macro invocation (in which case we push the resulting
                // invocation onto the input stack), and then reading a token
                // from whatever stream is on top of the stack.
                maybeBeginMacroInvocation();

                const auto result = lookaheadToken_;
                lookaheadToken_ = readTokenImpl();
                return result;
            }

            Token PeekToken() override
            {
                maybeBeginMacroInvocation();
                return lookaheadToken_;
            }

            // The "raw" read operations on an expansion input strema bypass
            // macro expansion and just read whatever token is next in the
            // input. These are useful for the top-level input stream of
            // a file, since we often want to read unexpanded tokens for
            // preprocessor directives.
            Token ReadRawToken()
            {
                const auto result = lookaheadToken_;
                lookaheadToken_ = readTokenImpl();
                return result;
            }

            Token PeekRawToken() const { return lookaheadToken_; }
            Token::Type PeekRawTokenType() const { return PeekRawToken().type; }

        private:
            /// Read a token, bypassing lookahead
            Token readTokenImpl() { return inputStreams_.ReadToken(); }

            /// Look at current input state and decide whether it represents a macro invocation
            void maybeBeginMacroInvocation();

            /// Parse one argument to a macro invocation
            MacroInvocation::Arg parseMacroArg(const std::shared_ptr<MacroInvocation>& macroInvocation);

            /// Parse all arguments to a macro invocation
            void parseMacroArgs(
                const std::shared_ptr<MacroDefinition>& macro,
                const std::shared_ptr<MacroInvocation>& macroInvocation);

            /// Push the given macro invocation into the stack of input streams
            void pushMacroInvocation(const std::shared_ptr<MacroInvocation>& macroInvocation);

        private:
            /// The base stream that macro expansion is being applied to
            std::shared_ptr<InputStream> base_;

            /// A stack of the base stream and active macro invocation in flight
            InputStreamStack inputStreams_;

            /// One token of lookahead
            Token lookaheadToken_;

            /// Token that "iniating" macro invocation in cases where multiple
            /// nested macro invocations might be in flight.
            Token initiatingMacroToken_;
        };

        // The top-level flow of the preprocessor is that it processed *input files*
        // An input file manages both the expansion of lexed tokens
        // from the source file, and also state related to preprocessor
        // directives, including skipping of code due to `#if`, etc.
        //
        // Input files are a bit like token streams, but they don't fit neatly into
        // the same abstraction due to all the special-case handling that directives
        // and conditionals require.
        struct InputFile : Common::NonCopyable
        {
        public:
            InputFile(const PreprocessorImpl& preprocessorImpl, const std::shared_ptr<SourceView>& sourceView)
            {
                ASSERT(sourceView);
                lexerStream_ = std::make_shared<LexerInputStream>(preprocessorImpl, sourceView);
                expansionInputStream_ = std::make_shared<ExpansionInputStream>(preprocessorImpl, lexerStream_);
            }

            /// Is this input file skipping tokens (because the current location is inside a disabled condition)?
            bool IsSkipping() const
            {
                // If we are not inside a preprocessor conditional, then don't skip
                const auto conditional = conditional_;
                if (!conditional)
                    return false;

                // skip tokens unless the conditional is inside its `true` case
                return conditional->state != Conditional::State::During;
            }

            /// Get the inner-most conditional that is in efffect at the current location
            std::shared_ptr<Conditional> GetInnerMostConditional() { return conditional_; }

            /// Push a new conditional onto the stack of conditionals in effect
            void PushConditional(const std::shared_ptr<Conditional>& conditional)
            {
                conditional->parent = conditional_;
                conditional_ = conditional;
            }

            /// Pop the inner-most conditional
            void PopConditional()
            {
                const auto conditional = conditional_;
                ASSERT(conditional);

                conditional_ = conditional->parent;
            }

            /// Read one token using all the expansion and directive-handling logic
            inline Token ReadToken() { return expansionInputStream_->ReadToken(); }

            inline Lexer& GetLexer() const { return lexerStream_->GetLexer(); }
            inline std::shared_ptr<SourceView> GetSourceView() const { return GetLexer().GetSourceView(); }
            inline std::shared_ptr<ExpansionInputStream> GetExpansionStream() const { return expansionInputStream_; }

        private:
            friend class PreprocessorImpl;

            /// The next outer input file
            ///
            /// E.g., if this file was `#include`d from another file, then `parent_` would be
            /// the file with the `#include` directive.
            std::shared_ptr<InputFile> parent_;

            /// The inner-most preprocessor conditional active for this file.
            std::shared_ptr<Conditional> conditional_;

            /// The lexer input stream that unexpanded tokens will be read from
            std::shared_ptr<LexerInputStream> lexerStream_;

            /// An input stream that applies macro expansion to `lexerStream_`
            std::shared_ptr<ExpansionInputStream> expansionInputStream_;
        };

        namespace
        {
            std::shared_ptr<InputFile> getInputFile(const DirectiveContext& context)
            {
                return context.inputFile;
            }

            // Wrapper for use inside directives
            inline bool isSkipping(const DirectiveContext& context)
            {
                return getInputFile(context)->IsSkipping();
            }
        }

        const std::unordered_map<std::string, PreprocessorImpl::Directive> PreprocessorImpl::directiveMap = {
            {"if", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleIfDirective}},
            {"ifdef", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleIfdefDirective}},
            {"ifndef", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleIfndefDirective}},
            {"else", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleElseDirective}},
            {"elif", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleElifDirective}},
            {"endif", {Directive::Flags::ProcessWhenSkipping, &PreprocessorImpl::handleEndIfDirective}},
            {"include", {Directive::Flags::None, &PreprocessorImpl::handleIncludeDirective}},
            {"define", {Directive::Flags::None, &PreprocessorImpl::handleDefineDirective}},
            {"undef", {Directive::Flags::None, &PreprocessorImpl::handleUndefDirective}},
            {"warning", {Directive::Flags::DontConsumeDirectiveAutomatically, &PreprocessorImpl::handleWarningDirective}},
            {"error", {Directive::Flags::DontConsumeDirectiveAutomatically, &PreprocessorImpl::handleErrorDirective}},
            {"line", {Directive::Flags::None, &PreprocessorImpl::handleLineDirective}},
            {"pragma", {Directive::Flags::None, &PreprocessorImpl::handlePragmaDirective}},
        };

        const std::unordered_map<std::string, PreprocessorImpl::PragmaDirective> PreprocessorImpl::paragmaDirectiveMap = {
            {"once", {&PreprocessorImpl::handlePragmaOnceDirective}},
        };

        PreprocessorImpl::PreprocessorImpl(const std::shared_ptr<IncludeSystem>& includeSystem,
                                           const std::shared_ptr<SourceManager>& sourceManager,
                                           const std::shared_ptr<CompileContext>& context)
            : includeSystem_(includeSystem),
              sourceManager_(sourceManager),
              context_(context)
        {
            ASSERT(context);
            ASSERT(includeSystem);
            ASSERT(sourceManager);

            // Add builtin macros
            {
                const char* const builtinNames[] = {"__FILE__", "__LINE__"};
                const MacroDefinition::Opcode builtinOpcodes[] = {MacroDefinition::Opcode::BuiltinFile, MacroDefinition::Opcode::BuiltinLine};

                for (int i = 0; size_t(i) < std::size(builtinNames); i++)
                {
                    const auto& name = builtinNames[i];

                    MacroDefinition::Op op;
                    op.opcode = builtinOpcodes[i];

                    auto macro = std::make_shared<MacroDefinition>();
                    macro->flavor = MacroDefinition::Flavor::BuiltinObjectLike;
                    macro->name = name;
                    macro->ops.push_back(op);

                    macrosDefinitions_[name] = macro;
                }
            }

            endOfFileToken_.type = Token::Type::EndOfFile;
        }

        std::vector<Token> PreprocessorImpl::ReadAllTokens()
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
                    case Token::Type::InvalidCharacter:
                        break;
                }
            }
        }

        void PreprocessorImpl::DefineMacro(const std::string& macro)
        {
            auto delimiterPos = macro.find('=');

            if (delimiterPos != std::string::npos)
            {
                DefineMacro(macro.substr(0, delimiterPos), macro.substr(delimiterPos + 1));
                return;
            }

            DefineMacro(macro, "");
        }

        void PreprocessorImpl::DefineMacro(const std::string& key, const std::string& value)
        {
            PathInfo pathInfo = PathInfo::makeCommandLine();

            // No funtion like macro is allowed here.
            bool functionLike = key.find('(') != std::string::npos;
            ASSERT(!functionLike);
            if (functionLike)
                return;

            auto macro = std::make_shared<MacroDefinition>();
            macro->flavor = MacroDefinition::Flavor::ObjectLike;

            auto keyFile = GetSourceManager().CreateFileFromString(pathInfo, key);
            auto valueFile = GetSourceManager().CreateFileFromString(pathInfo, value);

            // Note that we don't need to pass a special source loc to identify that these are defined on the command line
            // because the PathInfo on the SourceFile, is marked 'command line'.
            auto keyView = GetSourceManager().CreateSourceView(keyFile);
            auto valueView = GetSourceManager().CreateSourceView(valueFile);

            // Use existing `Lexer` to generate a token stream.
            Lexer keyLexer(keyView, context_);
            const auto keyTokens = keyLexer.LexAllSemanticTokens();

            // Key must contain only one token and EOF
            ASSERT(keyTokens.size() == 2);

            const auto keyName = keyTokens[0].GetContentString();
            macro->nameToken = keyTokens[0];
            macro->name = keyName;

            // Use existing `Lexer` to generate a token stream.
            Lexer valueLexer(valueView, context_);
            macro->tokens = valueLexer.LexAllSemanticTokens();

            std::unordered_map<std::string, uint32_t> mapParamNameToIndex;
            parseMacroOps(macro, mapParamNameToIndex);

            macrosDefinitions_[keyName] = macro;
        }

        // Find the currently-defined macro of the given name, or return nullptr
        // TODO: in cpp 20, String can be replaced with string_view.
        std::shared_ptr<MacroDefinition> PreprocessorImpl::LookupMacro(const std::string& name) const
        {
            const auto& search = macrosDefinitions_.find(name);
            return search != macrosDefinitions_.end() ? search->second : nullptr;
        }

        int32_t PreprocessorImpl::tokenToInt(const Token& token, int radix)
        {
            ASSERT(token.type == Token::Type::IntegerLiteral);

            errno = 0;

            char* end;
            const auto result = std::strtol(token.stringSlice.data(), &end, radix);

            if (errno == ERANGE)
                GetSink().Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.stringSlice, "int32_t");

            if (end == token.stringSlice.data() + token.stringSlice.size())
                return result;

            GetSink().Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.stringSlice, radix);
            return 0;
        }

        uint32_t PreprocessorImpl::tokenToUInt(const Token& token, int radix)
        {
            ASSERT(token.type == Token::Type::IntegerLiteral);

            errno = 0;

            char* end;
            const auto result = std::strtoul(token.stringSlice.data(), &end, radix);

            if (errno == ERANGE)
                GetSink().Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "uint32_t");

            if (end == token.stringSlice.data() + token.stringSlice.size())
                return result;

            GetSink().Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.stringSlice, radix);
            return 0;
        }

        Token PreprocessorImpl::readToken()
        {
            for (;;)
            {
                auto inputFile = currentInputFile_;

                if (!inputFile)
                    return endOfFileToken_;

                auto expansionStream = inputFile->GetExpansionStream();

                Token token = peekRawToken();
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

                // otherwise, if we are currently in a skipping mode, then skip tokens
                if (inputFile->IsSkipping())
                {
                    expansionStream->ReadRawToken();
                    continue;
                }

                token = expansionStream->PeekToken();
                if (token.type == Token::Type::EndOfFile)
                {
                    popInputFile();
                    continue;
                }

                expansionStream->ReadToken();
                return token;
            }
        }

        void PreprocessorImpl::PushInputFile(const std::shared_ptr<InputFile>& inputFile)
        {
            ASSERT(inputFile);

            inputFile->parent_ = currentInputFile_;
            currentInputFile_ = inputFile;
        }

        void PreprocessorImpl::popInputFile(bool force)
        {
            const auto& inputFile = currentInputFile_;
            ASSERT(inputFile);

            // We expect the file to be at its end, so that the
            // next token read would be an end-of-file token.
            const auto& expansionStream = inputFile->GetExpansionStream();
            Token eofToken = expansionStream->PeekRawToken();

            // The #line directive forces the current input to be closed.
            // Updating the token type to close the current input correctly.
            if (force)
                eofToken.type = Token::Type::EndOfFile;

            ASSERT(eofToken.type == Token::Type::EndOfFile);

            // If there are any open preprocessor conditionals in the file, then
            // we need to diagnose them as an error, because they were not closed
            // at the end of the file.
            for (auto conditional = inputFile->GetInnerMostConditional(); conditional; conditional = conditional->parent)
            {
                GetSink().Diagnose(eofToken, Diagnostics::endOfFileInPreprocessorConditional);
                GetSink().Diagnose(conditional->ifToken, Diagnostics::seeDirective, conditional->ifToken.stringSlice);
            }

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

        Token PreprocessorImpl::peekToken()
        {
            ASSERT(currentInputFile_);

            return currentInputFile_->expansionInputStream_->PeekToken();
        }

        Token PreprocessorImpl::peekRawToken()
        {
            ASSERT(currentInputFile_);

            return currentInputFile_->expansionInputStream_->PeekRawToken();
        }

        Token PreprocessorImpl::advanceToken()
        {
            ASSERT(currentInputFile_);

            if (isEndOfLine())
                return peekRawToken();

            return currentInputFile_->expansionInputStream_->ReadToken();
        }

        Token PreprocessorImpl::advanceRawToken()
        {
            ASSERT(currentInputFile_);

            return currentInputFile_->expansionInputStream_->ReadRawToken();
        }

        void PreprocessorImpl::skipToEndOfLine()
        {
            while (!isEndOfLine())
                advanceRawToken();
        }

        bool PreprocessorImpl::isEndOfLine()
        {
            ASSERT(currentInputFile_);

            switch (currentInputFile_->expansionInputStream_->PeekRawTokenType())
            {
                case Token::Type::EndOfFile:
                case Token::Type::NewLine:
                    return true;

                default:
                    return false;
            }
        }

        void setLexerDiagnosticSuppression(const std::shared_ptr<InputFile>& inputFile, bool shouldSuppressDiagnostics)
        {
            ASSERT(inputFile);

            if (shouldSuppressDiagnostics)
            {
                inputFile->GetLexer().EnableFlags(Lexer::Flags::SuppressDiagnostics);
            }
            else
            {
                inputFile->GetLexer().DisableFlags(Lexer::Flags::SuppressDiagnostics);
            }
        }

        bool PreprocessorImpl::expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic)
        {
            Token dummyToken;
            return expect(context, expected, diagnostic, dummyToken);
        }

        bool PreprocessorImpl::expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken)
        {
            if (peekTokenType() != expected)
            {
                // Only report the first parse error within a directive
                if (!context.parseError)
                    GetSink().Diagnose(peekRawToken(), diagnostic, expected, context.GetName());

                context.parseError = true;
                return false;
            }

            outToken = advanceToken();
            return true;
        }

        bool PreprocessorImpl::expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic)
        {
            Token dummyToken;
            return expectRaw(context, expected, diagnostic, dummyToken);
        }

        bool PreprocessorImpl::expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken)
        {
            if (peekRawTokenType() != expected)
            {
                // Only report the first parse error within a directive
                if (!context.parseError)
                    GetSink().Diagnose(peekRawToken(), diagnostic, expected, context.GetName());

                context.parseError = true;
                return false;
            }

            outToken = advanceRawToken();
            return true;
        }

        void PreprocessorImpl::expectEndOfDirective(DirectiveContext& context)
        {
            if (context.haveDoneEndOfDirectiveChecks)
                return;

            context.haveDoneEndOfDirectiveChecks = true;

            if (!isEndOfLine())
            {
                // If we already saw a previous parse error, then don't
                // emit another one for the same directive.
                if (!context.parseError)
                    GetSink().Diagnose(peekRawToken(), Diagnostics::unexpectedTokensAfterDirective, context.GetName());

                skipToEndOfLine();
            }
        }

        void PreprocessorImpl::handleDirective()
        {
            ASSERT(peekRawTokenType() == Token::Type::Pound);

            // Skip the `#`
            advanceRawToken();

            // Create a context for parsing the directive
            DirectiveContext context;
            context.inputFile = currentInputFile_;

            // Try to read the directive name.
            context.token = peekRawToken();

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
                GetSink().Diagnose(context.token, Diagnostics::expectedPreprocessorDirectiveName);
                skipToEndOfLine();
                return;
            }

            // Look up the handler for the directive.
            const auto directive = findDirective(context.GetName());

            // If we are skipping disabled code, and the directive is not one
            // of the small number that need to run even in that case, skip it.
            if (isSkipping(context) && !Common::IsSet(directive.flags, PreprocessorImpl::Directive::Flags::ProcessWhenSkipping))
            {
                skipToEndOfLine();
                return;
            }

            if (!Common::IsSet(directive.flags, Directive::Flags::DontConsumeDirectiveAutomatically))
            {
                // Consume the directive name token.
                advanceRawToken();
            }

            // Call the directive-specific handler
            (this->*directive.function)(context);

            // We expect the directive callback to consume the entire line, so if
            // it hasn't that is a parse error.
            expectEndOfDirective(context);
        }

        // Handle an invalid directive
        void PreprocessorImpl::handleInvalidDirective(DirectiveContext& directiveContext)
        {
            GetSink().Diagnose(directiveContext.token, Diagnostics::unknownPreprocessorDirective, directiveContext.GetName());
            skipToEndOfLine();
        }

        // Evaluate one infix operation in a preprocessor
        // conditional expression
        PreprocessorExpressionValue PreprocessorImpl::evaluateInfixOp(
            const DirectiveContext& context,
            const Token& opToken,
            PreprocessorExpressionValue left,
            PreprocessorExpressionValue right)
        {
            switch (opToken.type)
            {
                default:
                    GetSink().Diagnose(opToken, Diagnostics::internalCompilerError);
                    return 0;
                    break;
                case Token::Type::OpDiv:
                {
                    if (right == 0)
                    {
                        if (!context.parseError)
                            GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);

                        return 0;
                    }
                    return left / right;
                }
                case Token::Type::OpMod:
                {
                    if (right == 0)
                    {
                        if (!context.parseError)
                            GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);

                        return 0;
                    }
                    return left % right;
                }
                    // clang-format off
                case Token::Type::OpMul:      return left *  right;
                case Token::Type::OpAdd:      return left +  right;
                case Token::Type::OpSub:      return left -  right;
                case Token::Type::OpLsh:      return left << right;
                case Token::Type::OpRsh:      return left >> right;
                case Token::Type::OpLess:     return left <  right ? 1 : 0;
                case Token::Type::OpGreater:  return left >  right ? 1 : 0;
                case Token::Type::OpLeq:      return left <= right ? 1 : 0;
                case Token::Type::OpGeq:      return left >= right ? 1 : 0;
                case Token::Type::OpEql:      return left == right ? 1 : 0;
                case Token::Type::OpNeq:      return left != right ? 1 : 0;
                case Token::Type::OpBitAnd:   return left & right;
                case Token::Type::OpBitOr:    return left | right;
                case Token::Type::OpBitXor:   return left ^ right;
                case Token::Type::OpAnd:      return left && right;
                case Token::Type::OpOr:       return left || right; // clang-format on
            }
        }

        /// Parse a preprocessor expression, or skip it if we are in a disabled conditional
        PreprocessorExpressionValue PreprocessorImpl::skipOrParseAndEvaluateExpression(DirectiveContext& directiveContext)
        {
            const auto& inputStream = getInputFile(directiveContext);

            // If we are skipping, we want to ignore the expression (including
            // anything in it that would lead to a failure in parsing).
            // We can simply treat the expression as `0` in this case, since its
            // value won't actually matter.
            if (inputStream->IsSkipping())
            {
                // Consume everything until the end of the line
                skipToEndOfLine();
                return 0;
            }

            // Otherwise, we will need to parse an expression and return
            // its evaluated value.
            return parseAndEvaluateExpression(directiveContext);
        }

        /// Parse a complete (infix) preprocessor expression, and return its value
        PreprocessorExpressionValue PreprocessorImpl::parseAndEvaluateExpression(DirectiveContext& context)
        {
            // First read in the left-hand side (or the whole expression in the unary case)
            const auto value = parseAndEvaluateUnaryExpression(context);

            // Try to read in trailing infix operators with correct precedence
            return parseAndEvaluateInfixExpressionWithPrecedence(context, value, 0);
        }

        // Parse a unary (prefix) expression inside of a preprocessor directive.
        PreprocessorExpressionValue PreprocessorImpl::parseAndEvaluateUnaryExpression(DirectiveContext& context)
        {
            switch (peekTokenType())
            {
                case Token::Type::EndOfFile:
                case Token::Type::NewLine:
                    GetSink().Diagnose(peekToken(), Diagnostics::syntaxErrorInPreprocessorExpression);
                    return 0;
                default: break;
            }

            auto token = advanceToken();
            switch (token.type)
            {
                // handle prefix unary ops
                case Token::Type::OpSub: return -parseAndEvaluateUnaryExpression(context);
                case Token::Type::OpNot: return !parseAndEvaluateUnaryExpression(context);
                case Token::Type::OpBitNot: return ~parseAndEvaluateUnaryExpression(context);

                // handle parenthized sub-expression
                case Token::Type::LParent:
                {
                    Token leftParen = token;
                    PreprocessorExpressionValue value = parseAndEvaluateExpression(context);
                    if (!expect(context, Token::Type::RParent, Diagnostics::expectedTokenInPreprocessorExpression))
                        GetSink().Diagnose(leftParen, Diagnostics::seeOpeningToken, leftParen.stringSlice);

                    return value;
                }

                case Token::Type::IntegerLiteral: return tokenToInt(token, 10);

                case Token::Type::Identifier:
                {
                    if (token.stringSlice == "defined")
                    {
                        // handle `defined(someName)`

                        // Possibly parse a `(`
                        Token leftParen;
                        if (peekRawTokenType() == Token::Type::LParent)
                            leftParen = advanceRawToken();

                        // Expect an identifier
                        Token nameToken;
                        if (!expectRaw(context, Token::Type::Identifier, Diagnostics::expectedTokenInDefinedExpression, nameToken))
                            return 0;

                        std::string name = nameToken.GetContentString();
                        // If we saw an opening `(`, then expect one to close
                        if (leftParen.type != Token::Type::Unknown)
                        {
                            if (!expectRaw(context, Token::Type::RParent, Diagnostics::expectedTokenInDefinedExpression))
                            {
                                GetSink().Diagnose(leftParen, Diagnostics::seeOpeningToken, leftParen.stringSlice);
                                return 0;
                            }
                        }

                        return LookupMacro(name) != nullptr;
                    }

                    // An identifier here means it was not defined as a macro (or
                    // it is defined, but as a function-like macro. These should
                    // just evaluate to zero (possibly with a warning)
                    GetSink().Diagnose(token, Diagnostics::undefinedIdentifierInPreprocessorExpression, token.stringSlice);
                    return 0;
                }

                default:
                    GetSink().Diagnose(token, Diagnostics::syntaxErrorInPreprocessorExpression);
                    return 0;
            }
        }

        // Parse the rest of an infix preprocessor expression with
        // precedence greater than or equal to the given `precedence` argument.
        // The value of the left-hand-side expression is provided as
        // an argument.
        // This is used to form a simple recursive-descent expression parser.
        PreprocessorExpressionValue PreprocessorImpl::parseAndEvaluateInfixExpressionWithPrecedence(
            DirectiveContext& context,
            PreprocessorExpressionValue left,
            int32_t precedence)
        {
            for (;;)
            {
                // Look at the next token, and see if it is an operator of
                // high enough precedence to be included in our expression
                const auto& opToken = peekToken();
                const auto opPrecedence = GetInfixOpPrecedence(opToken);

                // If it isn't an operator of high enough precedence, we are done.
                if (opPrecedence < precedence)
                    break;

                // Otherwise we need to consume the operator token.
                advanceToken();

                // Next we parse a right-hand-side expression by starting with
                // a unary expression and absorbing and many infix operators
                // as possible with strictly higher precedence than the operator
                // we found above.
                PreprocessorExpressionValue right = parseAndEvaluateUnaryExpression(context);
                for (;;)
                {
                    // Look for an operator token
                    Token rightOpToken = peekToken();
                    int rightOpPrecedence = GetInfixOpPrecedence(rightOpToken);

                    // If no operator was found, or the operator wasn't high
                    // enough precedence to fold into the right-hand-side,
                    // exit this loop.
                    if (rightOpPrecedence <= opPrecedence)
                        break;

                    // Now invoke the parser recursively, passing in our
                    // existing right-hand side to form an even larger one.
                    right = parseAndEvaluateInfixExpressionWithPrecedence(context, right, rightOpPrecedence);
                }

                // Now combine the left- and right-hand sides using
                // the operator we found above.
                left = evaluateInfixOp(context, opToken, left, right);
            }
            return left;
        }

        void updateLexerFlagsForConditionals(const std::shared_ptr<InputFile>& inputFile)
        {
            setLexerDiagnosticSuppression(inputFile, inputFile->IsSkipping());
        }

        /// Start a preprocessor conditional, with an initial enable/disable state.
        void PreprocessorImpl::beginConditional(
            const DirectiveContext& context,
            bool enable)
        {
            const auto& inputFile = getInputFile(context);

            auto conditional = std::make_shared<Conditional>();
            conditional->ifToken = context.token;

            // Set state of this condition appropriately.
            // Default to the "haven't yet seen a `true` branch" state.
            Conditional::State state = Conditional::State::Before;

            // If we are nested inside a `false` branch of another condition, then
            // we never want to enable, so we act as if we already *saw* the `true` branch.
            if (inputFile->IsSkipping())
                state = Conditional::State::After;

            // Otherwise, if our condition was true, then set us to be inside the `true` branch
            else if (enable)
                state = Conditional::State::During;

            conditional->state = state;

            // Push conditional onto the stack
            inputFile->PushConditional(conditional);

            updateLexerFlagsForConditionals(inputFile);
        }

        // Handle a `#if` directive
        void PreprocessorImpl::handleIfDirective(DirectiveContext& directiveContext)
        {
            // Read a preprocessor expression (if not skipping), and begin a conditional
            // based on the value of that expression.
            const auto value = skipOrParseAndEvaluateExpression(directiveContext);

            beginConditional(directiveContext, value != 0);
        }

        void PreprocessorImpl::handleIfdefDirective(DirectiveContext& directiveContext)
        {
            // Expect a raw identifier, so we can check if it is defined
            Token nameToken;
            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, nameToken))
                return;

            // Check if the name is defined.
            beginConditional(directiveContext, LookupMacro(nameToken.GetContentString()) != nullptr);
        }

        void PreprocessorImpl::handleIfndefDirective(DirectiveContext& directiveContext)
        {
            // Expect a raw identifier, so we can check if it is defined
            Token nameToken;
            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, nameToken))
                return;

            // Check if the name is not defined.
            beginConditional(directiveContext, LookupMacro(nameToken.GetContentString()) == nullptr);
        }

        void PreprocessorImpl::handleElseDirective(DirectiveContext& directiveContext)
        {
            const auto& inputFile = getInputFile(directiveContext);
            ASSERT(inputFile);

            // if we aren't inside a conditional, then error
            const auto& conditional = inputFile->GetInnerMostConditional();
            if (!conditional)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::directiveWithoutIf, directiveContext.GetName());
                return;
            }

            // if we've already seen a `#else`, then it is an error
            if (conditional->elseToken.type != Token::Type::Unknown)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::directiveAfterElse, directiveContext.GetName());
                GetSink().Diagnose(conditional->elseToken, Diagnostics::seeDirective);
                return;
            }
            conditional->elseToken = directiveContext.token;

            switch (conditional->state)
            {
                case Conditional::State::Before:
                    conditional->state = Conditional::State::During;
                    break;

                case Conditional::State::During:
                    conditional->state = Conditional::State::After;
                    break;

                default:
                    break;
            }

            updateLexerFlagsForConditionals(inputFile);
        }

        void PreprocessorImpl::handleElifDirective(DirectiveContext& directiveContext)
        {
            // Need to grab current input stream *before* we try to parse
            // the conditional expression.
            const auto& inputFile = getInputFile(directiveContext);
            ASSERT(inputFile);

            // HACK(tfoley): handle an empty `elif` like an `else` directive
            // This is the behavior expected by at least one input program.
            // We will eventually want to be pedantic about this.
            // even if t
            switch (peekRawTokenType())
            {
                case Token::Type::EndOfFile:
                case Token::Type::NewLine:
                    GetSink().Diagnose(directiveContext.token, Diagnostics::directiveExpectsExpression, directiveContext.GetName());
                    handleElseDirective(directiveContext);
                    return;
                default:
                    break;
            }

            PreprocessorExpressionValue value = parseAndEvaluateExpression(directiveContext);

            // if we aren't inside a conditional, then error
            const auto& conditional = inputFile->GetInnerMostConditional();
            if (!conditional)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::directiveWithoutIf, directiveContext.GetName());
                return;
            }

            // if we've already seen a `#else`, then it is an error
            if (conditional->elseToken.type != Token::Type::Unknown)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::directiveAfterElse, directiveContext.GetName());
                GetSink().Diagnose(conditional->elseToken, Diagnostics::seeDirective);
                return;
            }

            switch (conditional->state)
            {
                case Conditional::State::Before:
                    if (value)
                        conditional->state = Conditional::State::During;
                    break;

                case Conditional::State::During:
                    conditional->state = Conditional::State::After;
                    break;

                default:
                    break;
            }

            updateLexerFlagsForConditionals(inputFile);
        }

        // Handle a `#endif` directive
        void PreprocessorImpl::handleEndIfDirective(DirectiveContext& directiveContext)
        {
            const auto& inputFile = getInputFile(directiveContext);
            ASSERT(inputFile);

            // if we aren't inside a conditional, then error
            const auto& conditional = inputFile->GetInnerMostConditional();
            if (!conditional)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::directiveWithoutIf, directiveContext.GetName());
                return;
            }

            inputFile->PopConditional();
            updateLexerFlagsForConditionals(inputFile);
        }

        // Handle a `#define` directive
        void PreprocessorImpl::handleDefineDirective(DirectiveContext& directiveContext)
        {
            Token nameToken;
            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, nameToken))
                return;

            std::string name = nameToken.GetContentString();

            const auto& oldMacro = LookupMacro(name);
            if (oldMacro)
            {
                if (oldMacro->IsBuiltin())
                {
                    GetSink().Diagnose(nameToken, Diagnostics::builtinMacroRedefinition, name);
                }
                else
                {
                    GetSink().Diagnose(nameToken, Diagnostics::macroRedefinition, name);

                    if (oldMacro->GetNameToken().isValid())
                        GetSink().Diagnose(oldMacro->GetNameToken(), Diagnostics::seePreviousDefinitionOf, name);
                }
            }
            auto macro = std::make_shared<MacroDefinition>();
            std::unordered_map<std::string, uint32_t> mapParamNameToIndex;

            // If macro name is immediately followed (with no space) by `(`,
            // then we have a function-like macro
            auto maybeOpenParen = peekRawToken();
            if (maybeOpenParen.type == Token::Type::LParent && !Common::IsSet(maybeOpenParen.flags, Token::Flags::AfterWhitespace))
            {
                // This is a function-like macro, so we need to remember that
                // and start capturing parameters
                macro->flavor = MacroDefinition::Flavor::FunctionLike;

                advanceRawToken();

                // If there are any parameters, parse them
                if (peekRawTokenType() != Token::Type::RParent)
                {
                    for (;;)
                    {
                        // A macro parameter should follow one of three shapes:
                        //
                        //      NAME
                        //      NAME...
                        //      ...
                        //
                        // If we don't see an ellipsis ahead, we know we ought
                        // to find one of the two cases that starts with an
                        // identifier.
                        Token paramNameToken;
                        if (peekRawTokenType() != Token::Type::Ellipsis)
                        {
                            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInMacroParameters, paramNameToken))
                                break;
                        }

                        // Whether or not a name was seen, we allow an ellipsis
                        // to indicate a variadic macro parameter.
                        //
                        // Note: a variadic parameter, if any, should always be
                        // the last parameter of a macro, but we do not enforce
                        Token ellipsisToken;
                        MacroDefinition::Param param;
                        param.sourceLocation = paramNameToken.sourceLocation;

                        if (peekRawTokenType() == Token::Type::Ellipsis)
                        {
                            ellipsisToken = advanceRawToken();
                            param.isVariadic = true;
                        }

                        if (paramNameToken.type != Token::Type::Unknown)
                        {
                            // If we read an explicit name for the parameter, then we can use
                            // that name directly.
                            param.name = paramNameToken.GetContentString();
                        }
                        else
                        {
                            // If an explicit name was not read for the parameter, we *must*
                            // have an unnamed variadic parameter. We know this because the
                            // only case where the logic above doesn't require a name to
                            // be read is when it already sees an ellipsis ahead.
                            ASSERT(ellipsisToken.type != Token::Type::Unknown);

                            // Any unnamed variadic parameter is treated as one named `__VA_ARGS__`
                            param.name = "__VA_ARGS__";
                        }

                        // TODO(tfoley): The C standard seems to disallow certain identifiers
                        // (e.g., `defined` and `__VA_ARGS__`) from being used as the names
                        // of user-defined macros or macro parameters. This choice seemingly
                        // supports implementation flexibility in how the special meanings of
                        // those identifiers are handled.
                        //
                        // We could consider issuing diagnostics for cases where a macro or parameter
                        // uses such names, or we could simply provide guarantees about what those
                        // names *do* in the context of the preprocessor.

                        // Add the parameter to the macro being deifned
                        auto paramIndex = macro->params.size();
                        macro->params.push_back(param);

                        const auto paramName = param.name;
                        if (mapParamNameToIndex.find(paramName) != mapParamNameToIndex.end())
                        {
                            GetSink().Diagnose(param.sourceLocation, Diagnostics::duplicateMacroParameterName, paramName);
                        }
                        else
                        {
                            mapParamNameToIndex[paramName] = uint32_t(paramIndex);
                        }

                        // If we see `)` then we are done with arguments
                        if (peekRawTokenType() == Token::Type::RParent)
                            break;

                        expectRaw(directiveContext, Token::Type::Comma, Diagnostics::expectedTokenInMacroParameters);
                    }
                }

                expectRaw(directiveContext, Token::Type::RParent, Diagnostics::expectedTokenInMacroParameters);

                // Once we have parsed the macro parameters, we can perform the additional validation
                // step of checking that any parameters before the last parameter are not variadic.
                int32_t lastParamIndex = int32_t(macro->params.size()) - 1;
                for (int32_t i = 0; i < lastParamIndex; ++i)
                {
                    auto& param = macro->params[i];
                    if (!param.isVariadic)
                        continue;

                    GetSink().Diagnose(param.sourceLocation, Diagnostics::variadicMacroParameterMustBeLast, param.name);

                    // As a precaution, we will unmark the variadic-ness of the parameter, so that
                    // logic downstream from this step doesn't have to deal with the possibility
                    // of a variadic parameter in the middle of the parameter list.
                    param.isVariadic = false;
                }
            }
            else
            {
                macro->flavor = MacroDefinition::Flavor::ObjectLike;
            }

            macro->nameToken = nameToken;
            macro->name = nameToken.GetContentString();

            macrosDefinitions_[name] = macro;

            // consume tokens until end-of-line
            for (;;)
            {
                Token token = peekRawToken();
                switch (token.type)
                {
                    default:
                        // In the ordinary case, we just add the token to the definition,
                        // and keep consuming more tokens.
                        advanceRawToken();
                        macro->tokens.push_back(token);
                        continue;

                    case Token::Type::EndOfFile:
                    case Token::Type::NewLine:
                        // The end of the current line/file ends the directive, and serves
                        // as the end-of-file marker for the macro's definition as well.
                        token.type = Token::Type::EndOfFile;
                        macro->tokens.push_back(token);
                        break;
                }
                break;
            }

            // Resetting the AfterWhitespace flag for the first token because the whitespace delimiter
            // after the #define directive is not part of the macro.
            if (!macro->tokens.empty())
                macro->tokens.front().flags &= ~Token::Flags::AfterWhitespace;

            parseMacroOps(macro, mapParamNameToIndex);
        }

        void PreprocessorImpl::handleUndefDirective(DirectiveContext& directiveContext)
        {
            Token nameToken;

            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, nameToken))
                return;

            const auto& name = nameToken.GetContentString();
            const auto& macro = LookupMacro(name);

            if (!macro)
            {
                // name wasn't defined
                GetSink().Diagnose(nameToken, Diagnostics::macroNotDefined, name);
                return;
            }

            // name was defined, so remove it
            macrosDefinitions_.erase(name);
        }

        std::string PreprocessorImpl::readDirectiveMessage()
        {
            std::string result;

            // This is a hacky shoddy way to get an unprocessed message string, with proper handling of a few whitespace characters, etc.
            // But it should work fine, because the tokens in the directive must be from the same memory slice.
            const auto begin = peekRawToken().stringSlice.begin();
            auto end = peekRawToken().stringSlice.end();

            while (!isEndOfLine())
                end = advanceRawToken().stringSlice.end();

            return {begin, end};
        }

        // Handle a `#warning` directive
        void PreprocessorImpl::handleWarningDirective(DirectiveContext& directiveContext)
        {
            setLexerDiagnosticSuppression(getInputFile(directiveContext), true);

            // Consume the directive
            advanceRawToken();

            // Read the message.
            std::string message = readDirectiveMessage();

            setLexerDiagnosticSuppression(getInputFile(directiveContext), false);

            // Report the custom error.
            GetSink().Diagnose(directiveContext.token, Diagnostics::userDefinedWarning, message);
        }

        void PreprocessorImpl::handleErrorDirective(DirectiveContext& directiveContext)
        {
            setLexerDiagnosticSuppression(getInputFile(directiveContext), true);

            // Consume the directive
            advanceRawToken();

            // Read the message.
            std::string message = readDirectiveMessage();

            setLexerDiagnosticSuppression(getInputFile(directiveContext), false);

            // Report the custom error.
            GetSink().Diagnose(directiveContext.token, Diagnostics::userDefinedError, message);
        }

        // Handle a `#include` directive
        void PreprocessorImpl::handleIncludeDirective(DirectiveContext& directiveContext)
        {
            Token pathToken;
            if (!expect(directiveContext, Token::Type::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective, pathToken))
                return;

            std::string path = getFileNameTokenValue(pathToken);

            const auto& sourceLocation = directiveContext.token.sourceLocation;
            const auto& includedFromPathInfo = sourceLocation.GetSourceView()->GetSourceFile();

            // Find the path relative to the foundPath
            PathInfo filePathInfo;
            if (RR_FAILED(includeSystem_->FindFile(path, includedFromPathInfo->GetPathInfo().foundPath, filePathInfo)))
            {
                GetSink().Diagnose(pathToken, Diagnostics::includeFailed, path);
                return;
            }

            // We must have a uniqueIdentity to be compare
            if (!filePathInfo.hasUniqueIdentity())
            {
                GetSink().Diagnose(pathToken, Diagnostics::noUniqueIdentity, path);
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
            filePathInfo.foundPath = std::filesystem::path(filePathInfo.foundPath).lexically_normal().generic_u8string();

            std::shared_ptr<SourceFile> includedFile;

            if (RR_FAILED(GetSourceManager().LoadFile(filePathInfo, includedFile)))
            {
                GetSink().Diagnose(pathToken, Diagnostics::includeFailed, path);
                return;
            }

            // This is a new parse (even if it's a pre-existing source file), so create a new
            const auto includedView = GetSourceManager().CreateIncluded(includedFile, directiveContext.token);

            PushInputFile(std::make_shared<InputFile>(*this, includedView));
        }

        void PreprocessorImpl::handleLineDirective(DirectiveContext& directiveContext)
        {
            uint32_t line = 0;

            switch (peekTokenType())
            {
                case Token::Type::IntegerLiteral:
                    line = tokenToUInt(advanceToken(), 10);
                    break;

                // `#line` and `#line default` directives are not supported
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

            const auto& nextToken = peekRawToken();

            auto sourceLocation = nextToken.sourceLocation;

            // Start new source view from end of new line sequence.
            sourceLocation.raw = sourceLocation.raw + nextToken.stringSlice.length();
            sourceLocation.humaneSourceLoc = HumaneSourceLocation(line, 1);

            // Todo trash
            pathInfo = PathInfo::makeSplit(pathInfo.foundPath, pathInfo.uniqueIdentity);
            const auto sourceView = GetSourceManager().CreateSplited(sourceLocation, pathInfo);

            // Forced closing of the current current stream and start a new one
            popInputFile(true);
            PushInputFile(std::make_shared<InputFile>(*this, sourceView));
        }

        // Handle a `#pragma` directive
        void PreprocessorImpl::handlePragmaDirective(DirectiveContext& directiveContext)
        {
            // Try to read the sub-directive name.
            const auto& subDirectiveToken = peekRawToken();

            // The sub-directive had better be an identifier
            if (subDirectiveToken.type != Token::Type::Identifier)
            {
                GetSink().Diagnose(directiveContext.token, Diagnostics::expectedPragmaDirectiveName);
                skipToEndOfLine();
                return;
            }
            advanceRawToken();

            // Look up the handler for the sub-directive.
            const auto& subDirective = findPragmaDirective(subDirectiveToken.GetContentString());

            // Apply the sub-directive-specific callback
            (this->*subDirective.function)(directiveContext, subDirectiveToken);
        }

        void PreprocessorImpl::handleUnknownPragmaDirective(DirectiveContext& directiveContext, const Token& subDirectiveToken)
        {
            std::ignore = directiveContext;

            GetSink().Diagnose(subDirectiveToken, Diagnostics::unknownPragmaDirectiveIgnored, subDirectiveToken.stringSlice);
            skipToEndOfLine();

            return;
        }

        void PreprocessorImpl::handlePragmaOnceDirective(DirectiveContext& directiveContext, const Token& subDirectiveToken)
        {
            // We need to identify the path of the file we are preprocessing,
            // so that we can avoid including it again.
            // We are using the 'uniqueIdentity' to determine file identities.
            auto issuedFromPathInfo = directiveContext.inputFile->GetSourceView()->GetPathInfo();

            // Must have uniqueIdentity for a #pragma once to work
            if (!issuedFromPathInfo.hasUniqueIdentity())
            {
                GetSink().Diagnose(subDirectiveToken, Diagnostics::pragmaOnceIgnored);
                return;
            }

            pragmaOnceUniqueIdentities_.emplace(issuedFromPathInfo.uniqueIdentity);
        }

        void PreprocessorImpl::parseMacroOps(const std::shared_ptr<MacroDefinition>& macro,
                                             const std::unordered_map<std::string, uint32_t>& mapParamNameToIndex)
        {
            // Scan through the tokens to recognize the "ops" that make up
            // the macro body.
            uint32_t spanBeginIndex = 0;
            uint32_t cursor = 0;
            for (;;)
            {
                auto spanEndIndex = cursor;
                auto tokenIndex = cursor++;
                const Token& token = macro->tokens[tokenIndex];
                MacroDefinition::Op newOp;

                switch (token.type)
                {
                    default:
                        // Most tokens just continue our current span.
                        continue;

                    case Token::Type::Identifier:
                    {
                        auto paramName = token.GetContentString();

                        const auto search = mapParamNameToIndex.find(paramName);
                        if (search == mapParamNameToIndex.end())
                            continue;

                        newOp.opcode = MacroDefinition::Opcode::ExpandedParam;
                        newOp.index0 = tokenIndex;
                        newOp.index1 = search->second;
                        newOp.flags = token.flags;
                    }
                    break;

                    case Token::Type::Pound:
                    {
                        auto paramNameTokenIndex = cursor;
                        auto paramNameToken = macro->tokens[paramNameTokenIndex];
                        if (paramNameToken.type != Token::Type::Identifier)
                        {
                            GetSink().Diagnose(token, Diagnostics::expectedMacroParameterAfterStringize);
                            continue;
                        }

                        auto paramName = paramNameToken.GetContentString();

                        const auto search = mapParamNameToIndex.find(paramName);
                        if (search == mapParamNameToIndex.end())
                        {
                            GetSink().Diagnose(token, Diagnostics::expectedMacroParameterAfterStringize);
                            continue;
                        }

                        cursor++;

                        newOp.opcode = MacroDefinition::Opcode::StringizedParam;
                        newOp.index0 = tokenIndex;
                        newOp.index1 = search->second;
                    }
                    break;

                    case Token::Type::PoundPound:
                        if (macro->ops.size() == 0 && (spanBeginIndex == spanEndIndex))
                        {
                            GetSink().Diagnose(token, Diagnostics::tokenPasteAtStart);
                            continue;
                        }

                        if (macro->tokens[cursor].type == Token::Type::EndOfFile)
                        {
                            GetSink().Diagnose(token, Diagnostics::tokenPasteAtEnd);
                            continue;
                        }

                        newOp.opcode = MacroDefinition::Opcode::TokenPaste;
                        newOp.index0 = tokenIndex;
                        newOp.index1 = 0;

                        // Okay, we need to do something here!
                        break;

                    case Token::Type::EndOfFile:
                        break;
                }

                if (spanBeginIndex != spanEndIndex || ((token.type == Token::Type::EndOfFile) && (macro->ops.size() == 0)))
                {
                    MacroDefinition::Op spanOp;
                    spanOp.opcode = MacroDefinition::Opcode::RawSpan;
                    spanOp.index0 = spanBeginIndex;
                    spanOp.index1 = spanEndIndex;
                    macro->ops.push_back(spanOp);
                }
                if (token.type == Token::Type::EndOfFile)
                    break;

                macro->ops.push_back(newOp);
                spanBeginIndex = cursor;
            }

            size_t opCount = macro->ops.size();
            ASSERT(opCount != 0);
            for (size_t i = 1; i < opCount - 1; ++i)
            {
                if (macro->ops[i].opcode == MacroDefinition::Opcode::TokenPaste)
                {
                    if (macro->ops[i - 1].opcode == MacroDefinition::Opcode::ExpandedParam)
                        macro->ops[i - 1].opcode = MacroDefinition::Opcode::UnexpandedParam;
                    if (macro->ops[i + 1].opcode == MacroDefinition::Opcode::ExpandedParam)
                        macro->ops[i + 1].opcode = MacroDefinition::Opcode::UnexpandedParam;
                }
            }
        }

        Preprocessor::~Preprocessor() { }

        Preprocessor::Preprocessor(const std::shared_ptr<IncludeSystem>& includeSystem,
                                   const std::shared_ptr<SourceManager>& sourceManager,
                                   const std::shared_ptr<CompileContext>& context)
            : impl_(std::make_unique<PreprocessorImpl>(includeSystem, sourceManager, context))
        {
        }

        void Preprocessor::PushInputFile(const std::shared_ptr<SourceFile>& sourceFile)
        {
            const auto sourceView = impl_->GetSourceManager().CreateSourceView(sourceFile);
            impl_->PushInputFile(std::make_shared<InputFile>(*impl_, sourceView));
        }

        void Preprocessor::DefineMacro(const std::string& macro)
        {
            ASSERT(impl_);
            impl_->DefineMacro(macro);
        }

        void Preprocessor::DefineMacro(const std::string& key, const std::string& value)
        {
            ASSERT(impl_);
            impl_->DefineMacro(key, value);
        }

        std::vector<Token> Preprocessor::ReadAllTokens()
        {
            ASSERT(impl_);
            return impl_->ReadAllTokens();
        }

        MacroInvocation::MacroInvocation(
            const PreprocessorImpl& preprocessor,
            const std::shared_ptr<MacroDefinition>& macro,
            const SourceLocation& macroInvocationLoc,
            const Token& initiatingMacroToken)
            : InputStream(preprocessor),
              macro_(macro),
              macroInvocationLoc_(macroInvocationLoc),
              initiatingMacroToken_(initiatingMacroToken)
        {
            ASSERT(macro);
            firstBusyMacroInvocation_ = this;
        }

        void MacroInvocation::Prime(MacroInvocation* nextBusyMacroInvocation)
        {
            nextBusyMacroInvocation_ = nextBusyMacroInvocation;

            initCurrentOpStream();
            lookaheadToken_ = readTokenImpl();
        }

        Token MacroInvocation::readTokenImpl()
        {
            // The `MacroInvocation` type maintains an invariant that after each
            // call to `readTokenImpl`:
            //
            // * The `currentOpStreams_` stack will be non-empty
            //
            // * The input state in `currentOpStreams_` will correspond to the
            //   macro definition op at index `macroOpIndex_`
            //
            // * The next token read from `currentOpStreams_` will not be an EOF
            //   *unless* the expansion has reached the end of the macro invocaiton
            //
            // The first time `readTokenImpl()` is called, it will only be able
            // to rely on the weaker invariant guaranteed by `initCurrentOpStream()`:
            //
            // * The `currentOpStreams_` stack will be non-empty
            //
            // * The input state in `currentOpStreams_` will correspond to the
            //   macro definition op at index `macroOpIndex_`
            //
            // * The next token read from `currentOpStream_` may be an EOF if
            //   the current op has an empty expansion.
            //
            // In either of those cases, we can start by reading the next token
            // from the expansion of the current op.
            Token token = currentOpStreams_.ReadToken();
            auto tokenOpIndex = macroOpIndex_;

            // Once we've read that `token`, we need to work to establish or
            // re-establish our invariant, which we do by looping until we are
            // in a valid state.
            for (;;)
            {
                // At the start of the loop, we already have the weaker invariant
                // guaranteed by `initCurrentOpStream()`: the current op stream
                // is in a consistent state, but it *might* be at its end.
                //
                // If the current stream is *not* at its end, then we seem to
                // have the stronger invariant as well, and we can return.
                if (currentOpStreams_.PeekTokenType() != Token::Type::EndOfFile)
                {
                    // We know that we have tokens remaining to read from
                    // `currentOpStreams_`, and we thus expect that the
                    // `token` we just read must also be a non-EOF token.
                    //
                    // Note: This case is subtle, because this might be the first invocation
                    // of `readTokenImpl()` after the `initCurrentOpStream()` call
                    // as part of `Prime()`. It seems that if the first macro op had
                    // an empty expansion, then `token` might be the EOF for that op.
                    //
                    // That detail is handled below in the logic for switching to a new
                    // macro op.
                    ASSERT(token.type != Token::Type::EndOfFile);

                    // We can safely return with our invaraints intact, because
                    // the next attempt to read a token will read a non-EOF.
                    return token;
                }

                // Otherwise, we have reached the end of the tokens coresponding
                // to the current op, and we want to try to advance to the next op
                // in the macro definition.
                auto currentOpIndex = macroOpIndex_;
                auto nextOpIndex = currentOpIndex + 1;

                // However, if we are already working on the last op in the macro
                // definition, then the next op index is out of range and we don't
                // want to advance. Instead we will keep the state of the macro
                // invocation where it is: at the end of the last op, returning
                // EOF tokens forever.
                //
                // Note that in this case we do not care whether `token` is an EOF
                // or not, because we expect the last op to yield an EOF at the
                // end of the macro expansion.
                if (nextOpIndex == macro_->ops.size())
                    return token;

                // Because `currentOpStreams_` is at its end, we can pop all of
                // those streams to reclaim their memory before we push any new
                // ones.
                currentOpStreams_.PopAll();

                // Now we've commited to moving to the next op in the macro
                // definition, and we want to push appropriate streams onto
                // the stack of input streams to represent that op.
                macroOpIndex_ = nextOpIndex;
                auto const& nextOp = macro_->ops[nextOpIndex];

                // What we do depends on what the next op's opcode is.
                switch (nextOp.opcode)
                {
                    default:
                    {
                        // All of the easy cases are handled by `initCurrentOpStream()`
                        // which also gets invoked in the logic of `MacroInvocation::Prime()`
                        // to handle the first op in the definition.
                        //
                        // This operation will set up `currentOpStreams_` so that it
                        // accurately reflects the expansion of the op at index `macroOpIndex_`.
                        //
                        // What it will *not* do is guarantee that the expansion for that
                        // op is non-empty. We will thus continue the outer `for` loop which
                        // checks whether the current op (which we just initialized here) is
                        // already at its end.
                        initCurrentOpStream();

                        // Before we go back to the top of the loop, we need to deal with the
                        // important corner case where `token` might have been an EOF because
                        // the very first op in a macro body had an empty expansion, e.g.:
                        //
                        //      #define TWELVE(X) X 12 X
                        //      TWELVE()
                        //
                        // In this case, the first `X` in the body of the macro will expand
                        // to nothing, so once that op is set up by `_initCurrentOpStrem()`
                        // the `token` we read here will be an EOF.
                        //
                        // The solution is to detect when all preceding ops considered by
                        // this loop have been EOFs, and setting the value to the first
                        // non-EOF token read.
                        if (token.type == Token::Type::EndOfFile)
                        {
                            token = currentOpStreams_.ReadToken();
                            tokenOpIndex = macroOpIndex_;
                        }
                    }
                    break;

                    case MacroDefinition::Opcode::TokenPaste:
                    {
                        // The more complicated case is a token paste (`##`).
                        auto tokenPasteTokenIndex = nextOp.index0;
                        const auto& tokenPasteLoc = macro_->tokens[tokenPasteTokenIndex].sourceLocation;

                        // A `##` must always appear between two macro ops (whether literal tokens
                        // or macro parameters) and it is supposed to paste together the last
                        // token from the left op with the first token from the right op.
                        //
                        // We will accumulate the pasted token as a string and then re-lex it.
                        std::stringstream pastedContent;

                        // Note that this is *not* the same as saying that we paste together the
                        // last token the preceded the `##` with the first token that follows it.
                        // In particular, if you have `L ## R` and either `L` or `R` has an empty
                        // expansion, then the `##` should treat that operand as empty.
                        //
                        // As such, there's a few cases to consider here.

                        // TODO: An extremely special case is the gcc-specific extension that allows
                        // the use of `##` for eliding a comma when there are no arguments for a
                        // variadic paameter, e.g.:
                        //
                        //      #define DEBUG(VALS...) debugImpl(__FILE__, __LINE__, ## VALS)
                        //
                        // Without the `##`, that case would risk producing an expression with a trailing
                        // comma when invoked with no arguments (e.g., `DEBUG()`). The gcc-specific
                        // behavior for `##` in this case discards the comma instead if the `VALS`
                        // parameter had no arguments (which is *not* the same as having a single empty
                        // argument).
                        //
                        // We could implement matching behavior in ParseTools with special-case logic here, but
                        // doing so adds extra complexity so we may be better off avoiding it.
                        //
                        // The Microsoft C++ compiler automatically discards commas in a case like this
                        // whether or not `##` has been used, except when certain flags to enable strict
                        // compliance to standards are used. Emulating this behavior would be another option.
                        //
                        // Later version of the C++ standard add `__VA_OPT__(...)` which can be used to
                        // include/exclude tokens in an expansion based on whether or not any arguments
                        // were provided for a variadic parameter. This is a relatively complicated feature
                        // to try and replicate
                        //
                        // For ParseTools it may be simplest to solve this problem at the parser level, by allowing
                        // trailing commas in argument lists without error/warning. However, if we *do* decide
                        // to implement the gcc extension for `##` it would be logical to try to detect and
                        // intercept that special case here.

                        // If the `tokenOpIndex` that `token` was read from is the op right
                        // before the `##`, then we know it is the last token produced by
                        // the preceding op (or possibly an EOF if that op's expansion was empty).
                        if (tokenOpIndex == nextOpIndex - 1)
                        {
                            if (token.type != Token::Type::EndOfFile)
                            {
                                if (Common::IsSet(token.flags, Token::Flags::AfterWhitespace))
                                    pastedContent.put(' ');

                                pastedContent << token.GetContentString();
                            }
                        }
                        else
                        {
                            // Otherwise, the op that preceded the `##` was *not* the same op
                            // that produced `token`, which could only happen if that preceding
                            // op was one that was initialized by this loop and then found to
                            // have an empty expansion. As such, we don't need to add anything
                            // onto `pastedContent` in this case.
                        }

                        // Once we've dealt with the token to the left of the `##` (if any)
                        // we can turn our attention to the token to the right.
                        //
                        // This token will be the first token (if any) to be produced by whatever
                        // op follows the `##`. We will thus start by initialiing the `currentOpStrems_`
                        // for reading from that op.
                        macroOpIndex_++;
                        initCurrentOpStream();

                        // If the right operand yields at least one non-EOF token, then we need
                        // to append that content to our paste result.
                        Token rightToken = currentOpStreams_.ReadToken();
                        if (rightToken.type != Token::Type::EndOfFile)
                            pastedContent << rightToken.GetContentString();

                        // Now we need to re-lex the token(s) that resulted from pasting, which requires
                        // us to create a fresh source file to represent the paste result.
                        PathInfo pathInfo = PathInfo::makeTokenPaste();

                        const auto& sourceFile = preprocessor_->GetSourceManager().CreateFileFromString(pathInfo, pastedContent.str());
                        auto sourceView = preprocessor_->GetSourceManager().CreatePastedSourceView(sourceFile, initiatingMacroToken_);

                        Lexer lexer(sourceView, preprocessor_->GetContext());
                        auto lexedTokens = lexer.LexAllSemanticTokens();

                        // Resetting the AtStartOfLine flag for the first token,
                        // because it is not part of TokenPaste, this is a side effect of relexing.
                        lexedTokens.front().flags &= ~Token::Flags::AtStartOfLine;

                        // The `lexedTokens` will always contain at least one token, representing an EOF for
                        // the end of the lexed token squence.
                        //
                        // Because we have concatenated together the content of zero, one, or two different
                        // tokens, there are many cases for what the result could be:
                        //
                        // * The content could lex as zero tokens, followed by an EOF. This would happen if
                        //   both the left and right operands to `##` were empty.
                        //
                        // * The content could lex to one token, followed by an EOF. This could happen if
                        //   one operand was empty but not the other, or if the left and right tokens concatenated
                        //   to form a single valid token.
                        //
                        // * The content could lex to more than one token, for cases like `+` pasted with `-`,
                        //   where the result is not a valid single token.
                        //
                        // The first two cases are both considered valid token pastes, while the latter should
                        // be diagnosed as a warning, even if it is clear how we can handle it.
                        if (lexedTokens.size() > 2)
                            GetSink().Diagnose(tokenPasteLoc, Diagnostics::invalidTokenPasteResult, pastedContent.str());

                        // No matter what sequence of tokens we got, we can create an input stream to represent
                        // them and push it as the representation of the `##` macro definition op.
                        //
                        // Note: the stream(s) created for the right operand will be on the stack under the new
                        // one we push for the pasted tokens, and as such the input state is capable of reading
                        // from both the input stream for the `##` through to the input for the right-hand-side
                        // op, which is consistent with `macroOpIndex_`.
                        const auto& inputStream = std::make_shared<SingleUseInputStream>(*preprocessor_, lexedTokens);
                        currentOpStreams_.Push(inputStream);

                        // There's one final detail to cover before we move on. *If* we used `token` as part
                        // of the content of the token paste, *or* if `token` is an EOF, then we need to
                        // replace it with the first token read from the expansion.
                        //
                        // (Otherwise, the `##` is being initialized as part of advancing through ops with
                        // empty expansion to the right of the op for a non-EOF `token`)
                        if ((tokenOpIndex == nextOpIndex - 1) || token.type == Token::Type::EndOfFile)
                        {
                            // Note that `tokenOpIndex` is being set here to the op index for the
                            // right-hand operand to the `##`. This is appropriate for cases where
                            // you might have chained `##` ops:
                            //
                            //      #define F(X,Y,Z) X ## Y ## Z
                            //
                            // If `Y` expands to a single token, then `X ## Y` should be treated
                            // as the left operand to the `Y ## Z` paste.
                            token = currentOpStreams_.ReadToken();
                            tokenOpIndex = macroOpIndex_;
                        }

                        // At this point we are ready to head back to the top of the loop and see
                        // if our invariants have been re-established.
                    }
                    break;
                }
            }
        }

        void MacroInvocation::initCurrentOpStream()
        {
            // The job of this function is to make sure that `currentOpStreams_` is set up
            // to refelct the state of the op at `macroOpIndex_`.
            auto opIndex = macroOpIndex_;
            auto& op = macro_->ops[opIndex];

            // As one might expect, the setup logic to apply depends on the opcode for the op.
            switch (op.opcode)
            {
                default:
                    ASSERT_MSG(false, "unhandled macro opcode case");
                    break;

                case MacroDefinition::Opcode::RawSpan:
                {
                    // A raw span of tokens (no use of macro parameters, etc.) is easy enough
                    // to handle. The operands of the op give us the begin/end index of the
                    // tokens in the macro definition that we'd like to use.
                    auto beginTokenIndex = op.index0;
                    auto endTokenIndex = op.index1;

                    // Because the macro definition stores its definition tokens directly, we
                    // can simply construct a token reader for reading from the tokens in
                    // the chosen range, and push a matching input stream.
                    auto tokenBuffer = macro_->tokens.begin();
                    auto tokenReader = TokenReader(tokenBuffer + beginTokenIndex, tokenBuffer + endTokenIndex);

                    TokenList tokenList;
                    initPastedSourceViewForTokens(tokenReader, initiatingMacroToken_, tokenList);
                    const auto& stream = std::make_shared<SingleUseInputStream>(*preprocessor_, tokenList);

                    //   const auto& stream = std::make_shared<PretokenizedInputStream>(*preprocessor_, tokenReader);
                    currentOpStreams_.Push(stream);
                }
                break;

                case MacroDefinition::Opcode::UnexpandedParam:
                {
                    // When a macro parameter is referenced as an operand of a token paste (`##`)
                    // it is not subjected to macro expansion.
                    //
                    // In this case, the zero-based index of the macro parameter was stored in
                    // the `index1` operand to the macro op.
                    auto paramIndex = op.index1;

                    // We can look up the corresponding argument to the macro invocation,
                    // which stores a begin/end pair of indices into the raw token stream
                    // that makes up the macro arguments.
                    auto tokenReader = getArgTokens(paramIndex);

                    // Because expansion doesn't apply to this parameter reference, we can simply
                    // play back those tokens exactly as they appeared in the argument list.
                    const auto& stream = std::make_shared<PretokenizedInputStream>(*preprocessor_, tokenReader);
                    currentOpStreams_.Push(stream);
                }
                break;

                case MacroDefinition::Opcode::ExpandedParam:
                {
                    // Most uses of a macro parameter will be subject to macro expansion.
                    // The initial logic here is similar to the unexpanded case above.
                    auto paramIndex = op.index1;
                    auto tokenReader = getArgTokens(paramIndex);

                    //  auto sourceView = GetSourceManager().CreatePastedSourceView(initiatingMacroToken_.sourceLocation.GetSourceView()->GetSourceFile(),
                    //                                                              initiatingMacroToken_);

                    TokenList tokenList;
                    initPastedSourceViewForTokens(tokenReader, initiatingMacroToken_, tokenList);

                    if (!tokenList.empty())
                        tokenList.front().flags = op.flags;

                    std::shared_ptr<InputStream> stream = std::make_shared<SingleUseInputStream>(*preprocessor_, tokenList);
                    //  if (!tokenReader.IsAtEnd() && tokenReader.PeekToken().flags != op.flags)
                    /*
                    std::shared_ptr<InputStream> stream;
                    // When the macro is expanded, we must make sure that the flags of the first
                    // token from the expansion are equal to the flags of the token that initiates the macro.
                    if (!tokenReader.IsAtEnd() && tokenReader.PeekToken().flags != op.flags)
                    {
                        TokenList tokenList;

                        // Copy all tokens and modify flags of first token
                        for (bool first = true; !tokenReader.IsAtEnd(); first = false)
                        {
                            auto token = tokenReader.AdvanceToken();

                            // Reset flags for first token in macro expansion
                            if (first)
                                token.flags = op.flags;

                            tokenList.push_back(token);
                        }

                        // Every token list needs to be terminated with an EOF,
                        // so we will construct one that matches the location
                        // for the `token`.
                        Token eofToken;
                        eofToken.type = Token::Type::EndOfFile;
                        tokenList.push_back(eofToken);

                        stream = std::make_shared<SingleUseInputStream>(*preprocessor_, tokenList);
                    }
                    else
                    {
                        stream = std::make_shared<PretokenizedInputStream>(*preprocessor_, tokenReader);
                    }*/

                    // The only interesting addition to the unexpanded case is that we wrap
                    // the stream that "plays back" the argument tokens with a stream that
                    // applies macro expansion to them.
                    const auto& expansion = std::make_shared<ExpansionInputStream>(*preprocessor_, stream);
                    currentOpStreams_.Push(expansion);
                }
                break;

                case MacroDefinition::Opcode::StringizedParam:
                {
                    // A macro parameter can also be "stringized" in which case the (unexpanded)
                    // argument tokens will be concatenated and escaped to form the content of
                    // a string literal.
                    //
                    // Much of the initial logic is shared with the other parameter cases above.
                    auto tokenIndex = op.index0;
                    const auto& loc = macro_->tokens[tokenIndex].sourceLocation;

                    auto paramIndex = op.index1;
                    auto tokenReader = getArgTokens(paramIndex);

                    std::string string;
                    for (bool first = true; !tokenReader.IsAtEnd(); first = false)
                    {
                        auto token = tokenReader.AdvanceToken();

                        // Any whitespace between the tokens of argument must be collapsed into
                        // a single space character. Fortunately for us, the lexer has tracked
                        // for each token whether it was immediately preceded by whitespace,
                        // so we can check for whitespace that precedes any token except the first.
                        if (!first && Common::IsSet(token.flags, Token::Flags::AfterWhitespace))
                            string.push_back(' ');

                        // We need to rememember to apply escaping to the content of any tokens
                        // being pulled into the string. E.g., this would come up if we end up
                        // trying to stringize a literal like `"this"` because we need the resulting
                        // token to be `"\"this\""` which includes the quote characters in the string
                        // literal value.
                        StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, token.stringSlice, string);
                    }

                    // Once we've constructed the content of the stringized result, we need to push
                    // a new single-token stream that represents that content.
                    pushSingleTokenStream(Token::Type::StringLiteral, loc, initiatingMacroToken_.sourceLocation.humaneSourceLoc, string);
                }
                break;

                case MacroDefinition::Opcode::BuiltinLine:
                {
                    // This is a special opcode used only in the definition of the built-in `__LINE__` macro
                    // (note that *uses* of `__LINE__` do not map to this opcode; only the definition of
                    // `__LINE__` itself directly uses it).
                    //
                    // Most of the logic for generating a token from the current source location is wrapped up
                    // in a helper routine so that we don't need to duplicate it between this and the `__FILE__`
                    // case below.
                    //
                    // The only key details here are that we specify the type of the token (`IntegerLiteral`)
                    // and its content (the value of `loc.line`).
                    pushStreamForSourceLocBuiltin(Token::Type::IntegerLiteral, [=](std::string& string, const SourceLocation& loc) { string += std::to_string(loc.humaneSourceLoc.line); });
                }
                break;

                case MacroDefinition::Opcode::BuiltinFile:
                {
                    // The `__FILE__` case is quite similar to `__LINE__`, except for the type of token it yields,
                    // and the way it computes the desired token content.
                    pushStreamForSourceLocBuiltin(Token::Type::StringLiteral, [=](std::string& string, const SourceLocation& loc) { string = loc.GetSourceView()->GetPathInfo().foundPath; });
                }
                break;

                case MacroDefinition::Opcode::TokenPaste:
                    // Note: If we ever end up in this case for `Opcode::TokenPaste`, then it implies
                    // something went very wrong.
                    //
                    // A `##` op should not be allowed to appear as the first (or last) token in
                    // a macro body, and consecutive `##`s should be treated as a single `##`.
                    //
                    // When `initCurrentOpStream()` gets called it is either:
                    //
                    // * called on the first op in the body of a macro (can't be a token paste)
                    //
                    // * called on the first op *after* a `##` (can't be another `##`)
                    //
                    // * explicitly tests for an handles token pastes spearately
                    //
                    // If we end up hitting the error here, then `initCurrentOpStream()` is getting
                    // called in an inappropriate case.
                    ASSERT_MSG(false, "token paste op in macro expansion");
                    break;
            }
        }

        bool MacroInvocation::IsBusy(const std::shared_ptr<MacroDefinition>& macro, MacroInvocation* duringMacroInvocation)
        {
            for (auto busyMacroInvocation = duringMacroInvocation; busyMacroInvocation; busyMacroInvocation = busyMacroInvocation->nextBusyMacroInvocation_)
            {
                if (busyMacroInvocation->macro_ == macro)
                    return true;
            }
            return false;
        }

        void MacroInvocation::pushSingleTokenStream(Token::Type tokenType, const SourceLocation& tokenLoc, const HumaneSourceLocation& humaneSourceLocation, std::string const& content)
        {
            // The goal here is to push a token stream that represents a single token
            // with exactly the given `content`, etc.
            // We are going to keep the content alive using the allocator to store token content.
            auto& allocator = preprocessor_->GetAllocator();
            const auto allocated = (char*)allocator.Allocate(content.length());
            std::copy(content.begin(), content.end(), allocated);

            Token token;
            token.type = tokenType;
            token.stringSlice = UnownedStringSlice(allocated, allocated + content.length());
            token.sourceLocation = tokenLoc;
            token.sourceLocation.humaneSourceLoc = humaneSourceLocation;

            TokenList lexedTokens;
            lexedTokens.push_back(token);

            // Every token list needs to be terminated with an EOF,
            // so we will construct one that matches the location
            // for the `token`.
            Token eofToken;
            eofToken.type = Token::Type::EndOfFile;
            eofToken.sourceLocation = token.sourceLocation;
            eofToken.flags = Token::Flags::AfterWhitespace | Token::Flags::AtStartOfLine;
            lexedTokens.push_back(eofToken);

            const auto& inputStream = std::make_shared<SingleUseInputStream>(*preprocessor_, lexedTokens);
            currentOpStreams_.Push(inputStream);
        }

        template <typename F>
        void MacroInvocation::pushStreamForSourceLocBuiltin(Token::Type tokenType, F const& valueBuilder)
        {
            // The `__LINE__` and `__FILE__` macros will always expand based on
            // the "initiating" source location, which should come from the
            // top-level file instead of any nested macros being expanded.
            const auto initiatingLoc = initiatingMacroToken_.sourceLocation;

            // The `valueBuilder` provided by the caller will determine what the content
            // of the token will be based on the source location (either to generate the
            // `__LINE__` or the `__FILE__` value).
            std::string content;
            valueBuilder(content, initiatingLoc);

            // Next we constuct and push an input stream with exactly the token type and content we want.
            pushSingleTokenStream(tokenType, initiatingLoc, initiatingLoc.humaneSourceLoc, content); // Todo Pasted inititing loc initiatingMacroToken_
        }

        // Check whether the current token on the given input stream should be
        // treated as a macro invocation, and if so set up state for expanding
        // that macro.
        void ExpansionInputStream::maybeBeginMacroInvocation()
        {
            // We iterate because the first token in the expansion of one
            // macro may be another macro invocation.
            for (;;)
            {
                // The "next" token to be read is already in our `lookaheadToken_`
                // member, so we can simply inspect it.
                //
                // We also care about where that token came from (which input stream).
                const auto& token = PeekRawToken();

                // If the token is not an identifier, then it can't possibly name a macro.
                if (token.type != Token::Type::Identifier)
                    return;

                // We will look for a defined macro matching the name.
                //
                // If there isn't one this couldn't possibly be the start of a macro
                // invocation.
                const auto& name = token.GetContentString();
                const auto& macro = preprocessor_->LookupMacro(name);

                if (!macro)
                    return;

                // Now we get to the slightly trickier cases.
                //
                // *If* the identifier names a macro, but we are currently in the
                // process of expanding the same macro (possibly via multiple
                // nested expansions) then we don't want to expand it again.
                //
                // We determine which macros are currently being expanded
                // by looking at the input stream assocaited with that one
                // token of lookahead.
                //
                // Note: it is critical here that `inputStreams_.getTopStream()`
                // returns the top-most stream that was active when `lookaheadToken_`
                // was consumed. This means that an `InputStreamStack` cannot
                // "pop" an input stream that it at its end until after something
                // tries to read an additional token.
                auto activeStream = inputStreams_.GetTopStream();

                // Each input stream keeps track of a linked list of the `MacroInvocation`s
                // that are considered "busy" while reading from that stream.
                auto busyMacros = activeStream->GetFirstBusyMacroInvocation();

                // If the macro is busy (already being expanded), we don't try to expand
                // it again, becaues that would trigger recursive/infinite expansion.
                if (MacroInvocation::IsBusy(macro, busyMacros))
                    return;

                // At this point we know that the lookahead token names a macro
                // definition that is not busy. it is *very* likely that we are
                // going to be expanding a macro.
                //
                // If we aren't already expanding a macro (meaning that the
                // current stream tokens are being read from is the "base" stream
                // that expansion is being applied to), then we want to consider
                // the location of this invocation as the "initiating" macro
                // invocation location for things like `__LINE__` uses inside
                // of macro bodies.
                if (activeStream == base_)
                    initiatingMacroToken_ = token;

                // The next steps depend on whether or not we are dealing
                // with a funciton-like macro.
                switch (macro->flavor)
                {
                    default:
                    {
                        // Object-like macros (whether builtin or user-defined) are the easy case.
                        //
                        // We simply create a new macro invocation based on the macro definition,
                        // prime its input stream, and then push it onto our stack of active
                        // macro invocations.
                        //
                        // Note: the macros that should be considered "busy" during the invocation
                        // are all those that were busy at the time we read the name of the macro
                        // to be expanded.

                        /// Create a new expansion of `macro`
                        const auto& invocation = std::make_shared<MacroInvocation>(*preprocessor_,
                                                                                   macro,
                                                                                   token.sourceLocation,
                                                                                   initiatingMacroToken_);

                        invocation->Prime(busyMacros);
                        pushMacroInvocation(invocation);
                    }
                    break;

                    case MacroDefinition::Flavor::FunctionLike:
                    {
                        // The function-like macro case is more complicated, primarily because
                        // of the need to handle arguments. The arguments of a function-like
                        // macro are expected to be tokens inside of balanced `()` parentheses.
                        //
                        // One special-case rule of the C/C++ preprocessor is that if the
                        // name of a function-like macro is *not* followed by a `(`, then
                        // it will not be subject to macro expansion. This design choice is
                        // motivated by wanting to be able to create a macro that handles
                        // direct calls to some primitive, along with a true function that handles
                        // cases where it is used in other ways. E.g.:
                        //
                        //      extern int coolFunction(int x);
                        //
                        //      #define coolFunction(x) x^0xABCDEF
                        //
                        //      int x = coolFunction(3); // uses the macro
                        //      int (*functionPtr)(int) f = coolFunction; // uses the function
                        //
                        // While we don't expect users to make heavy use of this feature in ParseTools,
                        // it is worthwhile to try to stay compatible.
                        //
                        // Because the macro name is already in `lookaheadToken_`, we can peak
                        // at the underlying input stream to see if the next non-whitespace
                        // token after the lookahead is a `(`.
                        inputStreams_.SkipAllWhitespace();
                        Token maybeLeftParen = inputStreams_.PeekToken();
                        if (maybeLeftParen.type != Token::Type::LParent)
                        {
                            // If we see a token other then `(` then we aren't suppsoed to be
                            // expanding the macro after all. Luckily, there is no state
                            // that we have to rewind at this point, because we never committed
                            // to macro expansion or consumed any (non-whitespace) tokens after
                            // the lookahead.
                            //
                            // We can simply bail out of looking for macro invocations, and the
                            // next read of a token will consume the lookahead token (the macro
                            // name) directly.
                            return;
                        }

                        auto sink = preprocessor_->GetSink();

                        // If we saw an opening `(`, then we know we are starting some kind of
                        // macro invocation, although we don't yet know if it is well-formed.
                        const auto& invocation = std::make_shared<MacroInvocation>(*preprocessor_,
                                                                                   macro,
                                                                                   token.sourceLocation,
                                                                                   initiatingMacroToken_);

                        // We start by consuming the opening `(` that we checked for above.
                        const auto& leftParen = inputStreams_.ReadToken();
                        ASSERT(leftParen.type == Token::Type::LParent);

                        // Next we parse any arguments to the macro invocation, which will
                        // consist of `()`-balanced sequences of tokens separated by `,`s.
                        parseMacroArgs(macro, invocation);
                        const uint32_t argCount = uint32_t(invocation->GetArgCount());

                        // We expect th arguments to be followed by a `)` to match the opening
                        // `(`, and if we don't find one we need to diagnose the issue.
                        if (inputStreams_.PeekTokenType() == Token::Type::RParent)
                        {
                            inputStreams_.ReadToken();
                        }
                        else
                        {
                            sink.Diagnose(inputStreams_.PeekToken(), Diagnostics::expectedTokenInMacroArguments, Token::Type::RParent, inputStreams_.PeekTokenType());
                        }

                        // The number of arguments at the macro invocation site might not
                        // match the number of arguments declared for the macro. In this
                        // case we diagnose an issue *and* skip expansion of this invocation
                        // (it effectively expands to zero new tokens).
                        const uint32_t paramCount = uint32_t(macro->params.size());
                        if (!macro->IsVariadic())
                        {
                            // The non-variadic case is simple enough: either the argument
                            // count exactly matches the required parameter count, or we
                            // diagnose an error.
                            if (argCount != paramCount)
                            {
                                sink.Diagnose(leftParen, Diagnostics::wrongNumberOfArgumentsToMacro, paramCount, argCount);
                                return;
                            }
                        }
                        else
                        {
                            // In the variadic case, we only require arguments for the
                            // non-variadic parameters (all but the last one). In addition,
                            // we do not consider it an error to have more than the required
                            // number of arguments.
                            const int32_t requiredArgCount = paramCount - 1;
                            if (int32_t(argCount) < requiredArgCount)
                            {
                                sink.Diagnose(leftParen, Diagnostics::wrongNumberOfArgumentsToMacro, requiredArgCount, argCount);
                                return;
                            }
                        }

                        // Now that the arguments have been parsed and validated,
                        // we are ready to proceed with expansion of the macro invocation.
                        //
                        // The main subtle thing we have to figure out is which macros should be considered "busy"
                        // during the expansion of this function-like macro invocation.
                        //
                        // In the case of an object-like macro invocation:
                        //
                        //      1 + M + 2
                        //            ^
                        //
                        // Input will have just read in the `M` token that names the macro
                        // so we needed to consider whatever macro invocations had been in
                        // flight (even if they were at their end) when checking if `M`
                        // was busy.
                        //
                        // In contrast, for a function-like macro invocation:
                        //
                        //      1 + F ( A, B, C ) + 2
                        //                        ^
                        //
                        // We will have just read in the `)` from the argument list, but
                        // we don't actually need/want to worry about any macro invocation
                        // that might have yielded the `)` token, since expanding that macro
                        // again would *not* be able to lead to a recursive case.
                        //
                        // Instead, we really only care about the active stream that the
                        // next token would be read from.
                        auto nextStream = inputStreams_.GetNextStream();
                        auto busyMacrosForFunctionLikeInvocation = nextStream->GetFirstBusyMacroInvocation();

                        invocation->Prime(busyMacrosForFunctionLikeInvocation);
                        pushMacroInvocation(invocation);
                    }
                    break;
                }

                // Update the lookahead token and set flags corresponding to the macro invocation token flags.
                lookaheadToken_ = readTokenImpl();
                lookaheadToken_.flags = token.flags;
            }
        }

        void MacroInvocation::initPastedSourceViewForTokens(TokenReader& tokenReader, const Token& initiatingToken, TokenList& outTokenList) const
        {
            // TODO ???
            // Copy all tokens and modify flags of first token
            std::shared_ptr<const SourceView> baseSourceView;
            std::shared_ptr<const SourceView> pastedSourceView;

            while (!tokenReader.IsAtEnd())
            {
                auto token = tokenReader.AdvanceToken();

                if (token.sourceLocation.GetSourceView() != baseSourceView)
                {
                    baseSourceView = token.sourceLocation.GetSourceView();
                    pastedSourceView = GetSourceManager().CreatePastedSourceView(
                        baseSourceView,
                        initiatingToken);
                }

                token.sourceLocation.sourceView = pastedSourceView;
                outTokenList.push_back(token);
            }

            // Every token list needs to be terminated with an EOF,
            // so we will construct one that matches the location
            // for the `token`.
            Token eofToken;
            eofToken.type = Token::Type::EndOfFile;
            outTokenList.push_back(eofToken);
        }

        TokenReader MacroInvocation::getArgTokens(uint32_t paramIndex)
        {
            ASSERT(paramIndex >= 0);
            ASSERT(paramIndex < macro_->params.size());

            // How we determine the range of argument tokens for a parameter
            // depends on whether or not it is a variadic parameter.
            auto& param = macro_->params[paramIndex];
            const auto argTokens = argTokens_.begin();
            if (!param.isVariadic)
            {
                // The non-variadic case is, as expected, the simpler one.
                //
                // We expect that there must be an argument at the index corresponding
                // to the parameter, and we construct a `TokenReader` that will play
                // back the tokens of that argument.
                ASSERT(paramIndex < args_.size());
                auto arg = args_[paramIndex];

                return TokenReader(argTokens + arg.beginTokenIndex, argTokens + arg.endTokenIndex);
            }
            else
            {
                ASSERT(args_.size() > 0);
                // In the variadic case, it is possible that we have zero or more
                // arguments that will all need to be played back in any place where
                // the variadic parameter is referenced.
                //
                // The first relevant argument is the one at the index coresponding
                // to the variadic parameter, if any. The last relevant argument is
                // the last argument to the invocation, *if* there was a first
                // relevant argument.
                auto firstArgIndex = paramIndex;
                auto lastArgIndex = args_.size() - 1;

                // One special case is when there are *no* arguments coresponding
                // to the variadic parameter.
                if (firstArgIndex > lastArgIndex)
                {
                    // When there are no arguments for the varaidic parameter we will
                    // construct an empty token range that comes after the other arguments.
                    auto arg = args_[lastArgIndex];
                    return TokenReader(argTokens + arg.endTokenIndex, argTokens + arg.endTokenIndex);
                }

                // Because the `argTokens_` array includes the commas between arguments,
                // we can get the token sequence we want simply by making a reader that spans
                // all the tokens between the first and last argument (inclusive) that correspond
                // to the variadic parameter.
                auto firstArg = args_[firstArgIndex];
                auto lastArg = args_[lastArgIndex];
                return TokenReader(argTokens + firstArg.beginTokenIndex, argTokens + lastArg.endTokenIndex);
            }
        }

        /// Parse one macro argument and return it in the form of a macro
        ///
        /// Assumes as a precondition that the caller has already checked
        /// for a closing `)` or end-of-input token.
        ///
        /// Does not consume any closing `)` or `,` for the argument.
        MacroInvocation::Arg ExpansionInputStream::parseMacroArg(const std::shared_ptr<MacroInvocation>& macroInvocation)
        {
            // Create the argument, represented as a special flavor of macro
            MacroInvocation::Arg arg;
            arg.beginTokenIndex = uint32_t(macroInvocation->argTokens_.size());

            // We will now read the tokens that make up the argument.
            //
            // We need to keep track of the nesting depth of parentheses,
            // because arguments should only break on a `,` that is
            // not properly nested in balanced parentheses.
            int32_t nestingDepth = 0;
            for (;;)
            {
                arg.endTokenIndex = uint32_t(macroInvocation->argTokens_.size());

                inputStreams_.SkipAllWhitespace();
                const Token token = inputStreams_.PeekToken();
                macroInvocation->argTokens_.push_back(token);

                switch (token.type)
                {
                    case Token::Type::EndOfFile:
                        // End of input means end of the argument.
                        // It is up to the caller to diagnose the
                        // lack of a closing `)`.
                        return arg;

                    case Token::Type::RParent:
                        // If we see a right paren when we aren't nested
                        // then we are at the end of an argument.
                        if (nestingDepth == 0)
                            return arg;

                        // Otherwise we decrease our nesting depth, add
                        // the token, and keep going
                        nestingDepth--;
                        break;

                    case Token::Type::Comma:
                        // If we see a comma when we aren't nested
                        // then we are at the end of an argument
                        if (nestingDepth == 0)
                            return arg;

                        // Otherwise we add it as a normal token
                        break;

                    case Token::Type::LParent:
                        // If we see a left paren then we need to
                        // increase our tracking of nesting
                        nestingDepth++;
                        break;

                    default:
                        break;
                }

                // Add the token and continue parsing.
                inputStreams_.ReadToken();
            }
        }

        /// Parse the arguments to a function-like macro invocation.
        ///
        /// This function assumes the opening `(` has already been parsed,
        /// and it leaves the closing `)`, if any, for the caller to consume.
        void ExpansionInputStream::parseMacroArgs(
            const std::shared_ptr<MacroDefinition>& macro,
            const std::shared_ptr<MacroInvocation>& macroInvocation)
        {
            // There is a subtle case here, which is when a macro expects
            // exactly one non-variadic parameter, but the argument list is
            // empty. E.g.:
            //
            //      #define M(x) /* whatever */
            //
            //      M()
            //
            // In this case we should parse a single (empty) argument, rather
            // than issue an error because of there apparently being zero
            // arguments.
            //
            // In all other cases (macros that do not have exactly one
            // parameter, plus macros with a single variadic parameter) we
            // should treat an empty argument list as zero
            // arguments for the purposes of error messages (since that is
            // how a programmer is likely to view/understand it).
            const auto paramCount = macro->params.size();
            if (paramCount != 1 || macro->IsVariadic())
            {
                // If there appear to be no arguments because the next
                // token would close the argument list, then we bail
                // out immediately.
                switch (inputStreams_.PeekTokenType())
                {
                    case Token::Type::RParent:
                    case Token::Type::EndOfFile:
                        return;
                    default:
                        break;
                }
            }

            // Otherwise, we have one or more arguments.
            for (;;)
            {
                // Parse an argument.
                MacroInvocation::Arg arg = parseMacroArg(macroInvocation);
                macroInvocation->args_.push_back(arg);

                // After consuming one macro argument, we look at
                // the next token to decide what to do.
                switch (inputStreams_.PeekTokenType())
                {
                    case Token::Type::RParent:
                    case Token::Type::EndOfFile:
                        // if we see a closing `)` or the end of
                        // input, we know we are done with arguments.
                        return;

                    case Token::Type::Comma:
                        // If we see a comma, then we will
                        // continue scanning for more macro
                        // arguments.
                        ReadRawToken();
                        break;

                    default:
                        // Any other token represents a syntax error.
                        //
                        // TODO: We could try to be clever here in deciding
                        // whether to break out of parsing macro arguments,
                        // or whether to "recover" and continue to scan
                        // ahead for a closing `)`. For now it is simplest
                        // to just bail.
                        GetSink().Diagnose(inputStreams_.PeekToken(), Diagnostics::errorParsingToMacroInvocationArgument, paramCount, macro->GetName());
                        return;
                }
            }
        }

        void ExpansionInputStream::pushMacroInvocation(const std::shared_ptr<MacroInvocation>& expansion)
        {
            inputStreams_.Push(expansion);
        }
    }
}