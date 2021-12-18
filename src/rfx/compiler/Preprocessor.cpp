#include "Preprocessor.hpp"

#include "compiler/DiagnosticCore.hpp"
#include "compiler/Lexer.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            struct Preprocessor::DirectiveContext
            {
                Token token;
                bool parseError = false;
            };

            namespace
            {
                inline bool isDirective(TokenType type)
                {
                    return (type >= TokenType::NullDirective && type <= TokenType::PragmaDirective);
                }

                /*UnownedStringSlice getDirectiveName(const Preprocessor::DirectiveContext& context)
                {
                    ASSERT(isDirective(context.token.type))
                    return context.token.stringSlice;
                }*/
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
                typedef InputStream Super;

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

            struct Preprocessor::InputSource
            {
                InputSource(
                    // Preprocessor* preprocessor,
                    // SourceView* sourceView
                )
                {
                }

                ~InputSource() = default;

                /// Is this input file skipping tokens (because the current location is inside a disabled condition)?
                //  bool isSkipping();

                /// Get the inner-most conditional that is in efffect at the current location
                // Conditional* getInnerMostConditional() { return m_conditional; }

                /// Push a new conditional onto the stack of conditionals in effect
                /* void pushConditional(Conditional* conditional)
                {
                    conditional->parent = m_conditional;
                    m_conditional = conditional;
                }*/

                /// Pop the inner-most conditional
                /* void popConditional()
                {
                    auto conditional = m_conditional;
                    ASSERT(conditional);
                    m_conditional = conditional->parent;
                    delete conditional;
                }*/

                /// Read one token using all the expansion and directive-handling logic
                void readToken()
                {
                    // return m_expansionStream->readToken();
                }

                // Lexer* getLexer() { return m_lexerStream->getLexer(); }

                // ExpansionInputStream* getExpansionStream() { return m_expansionStream; }

            private:
                friend class Preprocessor;

                /// The parent preprocessor
                // Preprocessor* m_preprocessor = nullptr;

                /// The next outer input file
                ///
                /// E.g., if this file was `#include`d from another file, then `parent_` would be
                /// the file with the `#include` directive.
                ///
                std::shared_ptr<InputSource> parent_;

                /// The inner-most preprocessor conditional active for this file.
                // Conditional* m_conditional = nullptr;

                /// The lexer input stream that unexpanded tokens will be read from
                //  LexerInputStream* m_lexerStream;

                /// An input stream that applies macro expansion to `m_lexerStream`
                // ExpansionInputStream* m_expansionStream;
            };

            Preprocessor::~Preprocessor()
            {
            }

            Preprocessor::Preprocessor(const std::shared_ptr<SourceFile>& sourceFile, const std::shared_ptr<DiagnosticSink>& diagnosticSink)
                : sourceFile_(sourceFile),
                  sink_(diagnosticSink)
            {
                auto sourceView = SourceView::Create(sourceFile_, sourceFile_->GetPathInfo());

                lexer_ = std::make_unique<Lexer>(sourceView, diagnosticSink);
            }

            void Preprocessor::pushInputSource(const std::shared_ptr<InputSource>& InputSource)
            {
                InputSource->parent_ = currentInputSource_;
                currentInputSource_ = InputSource;
            }

            void Preprocessor::popInputSource()
            {
            }

            std::shared_ptr<std::vector<Token>> Preprocessor::ReadAllTokens()
            {
                auto tokens = std::make_shared<std::vector<Token>>();
                for (;;)
                {
                    Token token = ReadToken();

                    switch (token.type)
                    {
                        default:
                            tokens->push_back(token);
                            break;

                        case TokenType::EndOfFile:
                            // Note: we include the EOF token in the list,
                            // since that is expected by the `TokenList` type.
                            tokens->push_back(token);
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

            Token Preprocessor::ReadToken()
            {
                for (;;)
                {
                    /* auto InputSource = currentInputSource_;
                    if (!InputSource)
                        return preprocessor->endOfFileToken;

                    auto expansionStream = InputSource->getExpansionStream();

                    // Look at the next raw token in the input.
                    Token token = expansionStream->peekRawToken();     */
                    Token token = lexer_->GetNextToken();
                    if (token.type == TokenType::EndOfFile)
                    {
                        //  preprocessor->popInputSource();
                        //  continue;
                        return token;
                    }

                    if (isDirective(token.type))
                    {
                        DirectiveContext context;
                        context.token = token;

                        handleDirective(context);
                    }
                    return token;
                    /*
                    // If we have a directive (`#` at start of line) then handle it
                    if ((token.type == TokenType::Pound) && (token.flags & TokenFlag::AtStartOfLine))
                    {
                        // Skip the `#`
                        expansionStream->readRawToken();

                        // Create a context for parsing the directive
                        PreprocessorDirectiveContext directiveContext;
                        directiveContext.m_preprocessor = preprocessor;
                        directiveContext.m_parseError = false;
                        directiveContext.m_haveDoneEndOfDirectiveChecks = false;
                        directiveContext.m_InputSource = InputSource;

                        // Parse and handle the directive
                        HandleDirective(&directiveContext);
                        continue;
                    }

                    // otherwise, if we are currently in a skipping mode, then skip tokens
                    if (InputSource->isSkipping())
                    {
                        expansionStream->readRawToken();
                        continue;
                    }

                    token = expansionStream->peekToken();
                    if (token.type == TokenType::EndOfFile)
                    {
                        preprocessor->popInputSource();
                        continue;
                    }

                    expansionStream->readToken();*/
                    return token;
                }
            }

            TokenType Preprocessor::peekRawTokenType()
            {
                return TokenType::Unknown;
            }

            bool Preprocessor::expectRaw(DirectiveContext& context, TokenType expected, DiagnosticInfo const& diagnostic)
            {
                (void) context;
                (void) expected;
                (void) diagnostic;
             /*   if (peekRawTokenType() != expected)
                {
                    // Only report the first parse error within a directive
                    if (!context.parseError)
                    {
                        sink_->Diagnose(peekLoc(), diagnostic, expected, getDirectiveName(context));
                    }
                    context.parseError = true;
                    return false;
                }
                Token const& token = AdvanceRawToken(context);
                if (outToken)
                    *outToken = token;*/
                return true;
            }

            void Preprocessor::handleDirective(DirectiveContext& directiveContext)
            {
                ASSERT(isDirective(directiveContext.token.type))

                switch (directiveContext.token.type)
                {
                    case TokenType::DefineDirective:
                        handleDefineDirective(directiveContext);
                        return;

                    default:
                        return;
                }
            }

            void Preprocessor::handleDefineDirective(DirectiveContext& directiveContext)
            {
                ASSERT(directiveContext.token.type == TokenType::DefineDirective)

                Token nameToken;
                if (!expectRaw(directiveContext, TokenType::Identifier, Diagnostics::expectedTokenInPreprocessorDirective))
                    return;
                // Name* name = nameToken.getName();
            }

        }
    }
}