#include "Preprocessor.hpp"

#include "compiler/DiagnosticCore.hpp"
#include "compiler/Lexer.hpp"

#include "core/IncludeSystem.hpp"
#include "core/StringEscapeUtil.hpp"

#include "common/LinearAllocator.hpp"

#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace RR
{
    namespace Rfx
    {
        namespace
        {
            struct DirectiveContext
            {
                Token token;
                bool parseError = false;
                bool haveDoneEndOfDirectiveChecks = false;
            };

            // Get the name of the directive being parsed.
            U8String getDirectiveName(const DirectiveContext& context)
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

        struct MacroInvocation;

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
            /// At the end of the stream should return a token with `Token::Type::EndOfFile`.
            ///
            virtual Token ReadToken() = 0;

            /// Peek at the next token in the input stream
            ///
            /// This function should return whatever `readToken()` will return next.
            ///
            /// At the end of the stream should return a token with `Token::Type::EndOfFile`.
            ///
            virtual Token PeekToken() = 0;

            // Based on `peekToken()` we can define a few more utility functions
            // for cases where we only care about certain details of the input.

            /// Peek the type of the next token in the input stream.
            Token::Type PeekTokenType() { return PeekToken().type; }

            /// Peek the location of the next token in the input stream.
            SourceLocation PeekLoc() { return PeekToken().sourceLocation; }

            /// Get the diagnostic sink to use for messages related to this stream
            // DiagnosticSink* getSink();

            std::shared_ptr<InputStream> GetParent() { return parent_; }

            void SetParent(const std::shared_ptr<InputStream>& parent) { parent_ = parent; }

            MacroInvocation* GetFirstBusyMacroInvocation() { return m_firstBusyMacroInvocation; }

            virtual void ForceClose() = 0;

        protected:
            /// The preprocessor that this input stream is being used by
            // Preprocessor* m_preprocessor = nullptr;

            /// Parent stream in the stack of secondary input streams
            std::shared_ptr<InputStream> parent_;

            /// Macro expansions that should be considered "busy" during expansion of this stream
            MacroInvocation* m_firstBusyMacroInvocation = nullptr; // TODO shoud it be shared_ptr ?
        };

        // During macro expansion, or the substitution of parameters into a macro body
        // we end up needing to track multiple active input streams, and this is most
        // easily done by having a distinct type to represent a stack of input streams.

        /// A stack of input streams, that will always read the next available token from the top-most stream
        ///
        /// An input stream stack assumes ownership of all streams pushed onto it, and will clean them
        /// up when they are no longer active or when the stack gets destructed.
        struct InputStreamStack
        {
            InputStreamStack()
            {
            }

            /// Clean up after an input stream stack
            ~InputStreamStack()
            {
                PopAll();
            }

            /// Push an input stream onto the stack
            void Push(const std::shared_ptr<InputStream>& stream)
            {
                stream->SetParent(m_top);
                m_top = stream;
            }

            /// Pop all input streams on the stack
            void PopAll()
            {
                m_top = nullptr;
            }

            /// Read a token from the top-most input stream with input
            ///
            /// If there is no input remaining, will return the EOF token
            /// of the bottom-most stream.
            ///
            /// At least one input stream must have been `push()`ed before
            /// it is valid to call this operation.
            Token ReadToken()
            {
                ASSERT(m_top)
                for (;;)
                {
                    // We always try to read from the top-most stream, and if
                    // it is not at its end, then we return its next token.
                    auto token = m_top->ReadToken();
                    if (token.type != Token::Type::EndOfFile)
                        return token;

                    // If the top stream has run out of input we try to
                    // switch to its parent, if any.
                    auto parent = m_top->GetParent();
                    if (parent)
                    {
                        // This stack has taken ownership of the streams,
                        // and must therefore delete the top stream before
                        // popping it.
                        m_top = parent;
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
            ///
            Token PeekToken()
            {
                // The logic here mirrors `readToken()`, but we do not
                // modify the `m_top` value or delete streams when they
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
                auto top = m_top;
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
            Token::Type PeekTokenType()
            {
                return PeekToken().type;
            }

            /// Return location of the token that `peekToken()` will return
            /* SourceLoc PeekLoc()
            {
                return PeekToken().loc;
            }*/

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
            std::shared_ptr<InputStream> GetTopStream()
            {
                return m_top;
            }

            /// Get the input stream that the next token would come from
            ///
            /// If the input stack is at its end, this will just be the top-most stream.
            std::shared_ptr<InputStream> GetNextStream()
            {
                ASSERT(m_top);
                auto top = m_top;
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
            std::shared_ptr<InputStream> m_top = nullptr;
        };

        // The simplest types of input streams are those that simply "play back"
        // a list of tokens that was already captures. These types of streams
        // are primarily used for playing back the tokens inside of a macro body.

        /// An input stream that reads from a list of tokens that had already been tokenized before.
        struct PretokenizedInputStream : InputStream
        {
        public:
            /// Initialize an input stream with list of `tokens`
            PretokenizedInputStream(TokenReader const& tokens)
                : tokenReader_(tokens)
            {
            }

            // A pretokenized stream implements the key read/peek operations
            // by delegating to the underlying token reader.
            virtual Token ReadToken() override
            {
                return tokenReader_.AdvanceToken();
            }

            virtual Token PeekToken() override
            {
                return tokenReader_.PeekToken();
            }

            virtual void ForceClose() override
            {
                return; // Do nothing. TODO This is begin looking wierd
            }

        protected:
            PretokenizedInputStream() {};

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
            typedef PretokenizedInputStream Super;

            SingleUseInputStream(const TokenList& lexedTokens)
                : PretokenizedInputStream(), lexedTokens_(lexedTokens)
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

            void ForceClose() override
            {
                lookaheadToken_ = Token(Token::Type::EndOfFile, UnownedStringSlice(nullptr, nullptr), lookaheadToken_.sourceLocation, lookaheadToken_.humaneSourceLocation);
                isClosed_ = true;
            }

        private:
            /// Read a token from the lexer, bypassing lookahead
            Token readTokenImpl()
            {
                if (isClosed_)
                    return lookaheadToken_;

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

            bool isClosed_ = false;
        };

        // TODO comment
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
            };

            struct Param
            {
                //   NameLoc nameLoc;
                bool isVariadic = false;
            };

        public:
            U8String GetName()
            {
                return name;
            }

            Token GetNameToken()
            {
                return nameToken;
            }
            /*
            SourceLoc GetLoc()
            {
                return nameAndLoc.loc;
            }*/

            bool IsBuiltin()
            {
                return flavor == MacroDefinition::Flavor::BuiltinObjectLike;
            }

            /// Is this a variadic macro?
            /* bool IsVariadic()
            {
                // A macro is variadic if it has a last parameter and
                // that last parameter is a variadic parameter.
                //
                auto paramCount = params.getCount();
                if (paramCount == 0)
                    return false;
                return params[paramCount - 1].isVariadic;
            }*/
        public:
            //TODO RENAME?

            /// The flavor of macro
            MacroDefinition::Flavor flavor;

            /// The name under which the macro was `#define`d
            /// TODO: replace name with uniqueidentifier for better performance
            U8String name;

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
                const std::shared_ptr<PreprocessorImpl>& preprocessor,
                const std::shared_ptr<DiagnosticSink>& sink,
                const std::shared_ptr<MacroDefinition>& macro, // Todo shared_ptr?
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

            virtual Token PeekToken() override
            {
                return lookaheadToken_;
            }

            void ForceClose() override
            {
                lookaheadToken_ = Token(Token::Type::EndOfFile, UnownedStringSlice(nullptr, nullptr), lookaheadToken_.sourceLocation, lookaheadToken_.humaneSourceLocation);
                isClosed_ = true;
            }

            /// Is the given `macro` considered "busy" during the given macroinvocation?
            static bool IsBusy(const std::shared_ptr<MacroDefinition>& macro, MacroInvocation* duringMacroInvocation);

            //   uint32_t getArgCount() { return m_args.getCount(); }

        private:
            // Macro invocations are created as part of applying macro expansion
            // to a stream, so the `ExpansionInputStream` type takes responsibility
            // for setting up much of the state of a `MacroInvocation`.
            // TODO
            // friend struct ExpansionInputStream;

            //TOOD COMMNETS
            std::shared_ptr<PreprocessorImpl> preprocessor_ = nullptr;
            std::shared_ptr<DiagnosticSink> sink_ = nullptr;

            /// The macro being expanded
            std::shared_ptr<MacroDefinition> m_macro = nullptr;

            /// A single argument to the macro invocation
            ///
            /// Each argument is represented as a begin/end pair of indices
            /// into the sequence of tokens that make up the macro arguments.
            ///
            struct Arg
            {
                uint32_t beginTokenIndex = 0;
                uint32_t endTokenIndex = 0;
            };

            /// Tokens that make up the macro arguments, in case of function-like macro expansion
            std::vector<Token> m_argTokens;

            /// Arguments to the macro, in the case of a function-like macro expansion
            std::vector<Arg> m_args;

            /// Additional macros that should be considered "busy" during this expansion
            MacroInvocation* m_nextBusyMacroInvocation = nullptr;

            /// Locatin of the macro invocation that led to this expansion
            SourceLocation m_macroInvocationLoc;

            /// "iniating" macro token invocation in cases where multiple
            /// nested macro invocations might be in flight.
            Token initiatingMacroToken_;

            /// One token of lookahead
            Token lookaheadToken_;

            /// Actually read a new token (not just using the lookahead)
            Token readTokenImpl();

            // In order to play back a macro definition, we will play back the ops
            // in its body one at a time. Each op may expand to a stream of zero or
            // more tokens, so we need some state to track all of that.

            /// One or more input streams representing the current "op" being expanded
            InputStreamStack m_currentOpStreams;

            /// The index into the macro's list of the current operation being played back
            uint32_t m_macroOpIndex = 0;

            /// Initialize the input stream for the current macro op
            void initCurrentOpStream();

            /// Get a reader for the tokens that make up the macro argument at the given `paramIndex`
            TokenReader getArgTokens(uint32_t paramIndex);

            /// Push a stream onto `m_currentOpStreams` that consists of a single token
            void pushSingleTokenStream(Token::Type tokenType, const SourceLocation& sourceLocation, U8String const& content);

            /// Push a stream for a source-location builtin (`__FILE__` or `__LINE__`), with content set up by `valueBuilder`
            template <typename F>
            void pushStreamForSourceLocBuiltin(Token::Type tokenType, F const& valueBuilder);

            bool isClosed_ = false;
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
            /// Construct an input stream that applies macro expansion to `base`
            ExpansionInputStream(
                const std::weak_ptr<PreprocessorImpl>& preprocessor,
                const std::shared_ptr<InputStream>& base)
                : preprocessor_(preprocessor), base_(base)
            {
                ASSERT(preprocessor.lock())

                inputStreams_.Push(base);
            }

            Token ReadToken() override
            {
                // Reading a token from an expansion strema amounts to checking
                // whether the current state of the input stream marks the start
                // of a macro invocation (in which case we push the resulting
                // invocation onto the input stack), and then reading a token
                // from whatever stream is on top of the stack.
                maybeBeginMacroInvocation();
                return readTokenImpl();
            }

            Token PeekToken() override
            {
                maybeBeginMacroInvocation();
                return inputStreams_.PeekToken();
            }

            // The "raw" read operations on an expansion input strema bypass
            // macro expansion and just read whatever token is next in the
            // input. These are useful for the top-level input stream of
            // a file, since we often want to read unexpanded tokens for
            // preprocessor directives.
            Token ReadRawToken()
            {
                return readTokenImpl();
            }

            Token PeekRawToken()
            {
                return inputStreams_.PeekToken();
            }

            Token::Type PeekRawTokenType() { return PeekRawToken().type; }

            void ForceClose() override
            {
                //TOOTODO
                // TODO
                return;
            }

        private:
            /// Read a token, bypassing lookahead
            Token readTokenImpl()
            {
                return inputStreams_.ReadToken();
            }

            /// Look at current input state and decide whether it represents a macro invocation
            void maybeBeginMacroInvocation();

            /// Parse one argument to a macro invocation
            //            MacroInvocation::Arg parseMacroArg(MacroInvocation* macroInvocation);

            /// Parse all arguments to a macro invocation
            /* void parseMacroArgs(
                MacroDefinition* macro,
                MacroInvocation* macroInvocation);
                */

            /// Push the given macro invocation into the stack of input streams
            void pushMacroInvocation(const std::shared_ptr<MacroInvocation>& macroInvocation);

        private:
            ///  TODO comment
            std::weak_ptr<PreprocessorImpl> preprocessor_;

            /// The base stream that macro expansion is being applied to
            std::shared_ptr<InputStream> base_ = nullptr;

            /// A stack of the base stream and active macro invocation in flight
            InputStreamStack inputStreams_;

            /// Token that "iniating" macro invocation in cases where multiple
            /// nested macro invocations might be in flight.
            Token initiatingMacroToken_;
        };

        class PreprocessorImpl final
        {
        public:
            // The top-level flow of the preprocessor is that it processed *input files*
            // An input file manages both the expansion of lexed tokens
            // from the source file, and also state related to preprocessor
            // directives, including skipping of code due to `#if`, etc. TODO COMMENT(if)
            //
            // Input files are a bit like token streams, but they don't fit neatly into
            // the same abstraction due to all the special-case handling that directives
            // and conditionals require.
            struct InputFile
            {
                InputFile(const std::weak_ptr<PreprocessorImpl>& preprocessorImpl, const std::shared_ptr<SourceView>& sourceView)
                {
                    ASSERT(sourceView)
                    ASSERT(preprocessorImpl.lock())

                    lexerStream_ = std::make_shared<LexerInputStream>(sourceView, preprocessorImpl.lock()->GetSink());
                    expansionInputStream_ = std::make_shared<ExpansionInputStream>(preprocessorImpl, lexerStream_);
                }

                /// Read one token using all the expansion and directive-handling logic
                Token ReadToken()
                {
                    return expansionInputStream_->ReadToken();
                }

                inline Lexer& GetLexer()
                {
                    return lexerStream_->GetLexer();
                }

                inline std::shared_ptr<ExpansionInputStream> GetExpansionStream()
                {
                    return expansionInputStream_;
                }

            private:
                friend class PreprocessorImpl;

                /// The next outer input file
                ///
                /// E.g., if this file was `#include`d from another file, then `m_parent` would be
                /// the file with the `#include` directive.
                std::shared_ptr<InputFile> parent_;

                /// The lexer input stream that unexpanded tokens will be read from
                std::shared_ptr<LexerInputStream> lexerStream_;

                /// An input stream that applies macro expansion to `lexerStream_`
                std::shared_ptr<ExpansionInputStream> expansionInputStream_;
            };

            typedef void (PreprocessorImpl::*HandleDirectiveFunc)(DirectiveContext& context);

            struct Directive
            {
                enum class Flags : uint32_t
                {
                    None = 0,
                    DontConsumeDirectiveAutomatically = 1 << 0,
                };

                Flags flags;
                HandleDirectiveFunc function;
            };

        public:
            PreprocessorImpl(const std::shared_ptr<IncludeSystem>& includeSystem,
                             const std::shared_ptr<DiagnosticSink>& diagnosticSink);

            std::vector<Token> ReadAllTokens();

            // Find the currently-defined macro of the given name, or return NULL
            std::shared_ptr<MacroDefinition> LookupMacro(const U8String& name) const;

            std::shared_ptr<DiagnosticSink> GetSink() const { return sink_; }
            std::shared_ptr<IncludeSystem> GetIncludeSystem() const { return includeSystem_; }
            std::shared_ptr<LinearAllocator> GetAllocator() const { return allocator_; }

            // Push a new input file onto the input stack of the preprocessor
            void PushInputFile(const std::shared_ptr<InputFile>& inputFile);

        private:
            int32_t tokenToInt(const Token& token, int radix);
            uint32_t tokenToUInt(const Token& str, int radix);

            Token readToken();

            // Pop the inner-most input file from the stack of input files
            void popInputFile();

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

            void setLexerDiagnosticSuppression(const std::shared_ptr<InputFile>& inputFile, bool shouldSuppressDiagnostics);

            bool expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken = dummyToken);
            bool expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken = dummyToken);

            U8String readDirectiveMessage();

            void handleDirective();
            void handleInvalidDirective(DirectiveContext& directiveContext);
            void handleDefineDirective(DirectiveContext& directiveContext);
            void handleUndefDirective(DirectiveContext& directiveContext);
            void handleWarningDirective(DirectiveContext& directiveContext);
            void handleIncludeDirective(DirectiveContext& directiveContext);
            void handleLineDirective(DirectiveContext& directiveContext);

            // Helper routine to check that we find the end of a directive where
            // we expect it.
            //
            // Most directives do not need to call this directly, since we have
            // a catch-all case in the main `handleDirective()` function.
            // `#include` and `#line` case will call it directly to avoid complications
            // when it switches the input stream.
            void expectEndOfDirective(DirectiveContext& context);

            void parseMacroOps(std::shared_ptr<MacroDefinition>& macro,
                               const std::unordered_map<U8String, uint32_t>& mapParamNameToIndex);

        private:
            std::shared_ptr<DiagnosticSink> sink_;
            std::shared_ptr<IncludeSystem> includeSystem_;
            std::shared_ptr<LinearAllocator> allocator_;

            /// A stack of "active" input files
            std::shared_ptr<InputFile> currentInputFile_;

            /// The unique identities of any paths that have issued `#pragma once` directives to
            /// stop them from being included again.
            std::unordered_set<U8String> pragmaOnceUniqueIdentities_;

            /// A pre-allocated token that can be returned to represent end-of-input situations.
            Token endOfFileToken_;

            /// Macros defined in this environment
            std::unordered_map<U8String, std::shared_ptr<MacroDefinition>> macrosDefinitions_;

        private:
            static Token dummyToken;
            static const std::unordered_map<U8String, Directive> directiveMap;

        private:
            // Look up the directive with the given name.
            static Directive findDirective(const U8String& name)
            {
                auto search = directiveMap.find(name);

                if (search == directiveMap.end())
                    return { Directive::Flags::None, &handleInvalidDirective };

                return search->second;
            }
        };

        Token PreprocessorImpl::dummyToken;

        const std::unordered_map<U8String, PreprocessorImpl::Directive> PreprocessorImpl::directiveMap = {
            //   { "if", { Directive::Flags::None, handleIfDirective } },
            //  { "ifdef", nullptr },
            //  { "ifndef", nullptr },
            //   { "else", nullptr },
            //  { "elif", nullptr },
            //  { "endif", nullptr },
            // { "include", &PreprocessorImpl::handleIncludeDirective },
            { "define", { Directive::Flags::None, &handleDefineDirective } },
            { "undef", { Directive::Flags::None, &handleUndefDirective } },
            { "warning", { Directive::Flags::DontConsumeDirectiveAutomatically, &handleWarningDirective } },
            //  { "error", nullptr },
            // { "line", &PreprocessorImpl::handleLineDirective },
            //   { "pragma", nullptr }
        };

        PreprocessorImpl::PreprocessorImpl(const std::shared_ptr<IncludeSystem>& includeSystem,
                                           const std::shared_ptr<DiagnosticSink>& diagnosticSink)
            : sink_(diagnosticSink),
              includeSystem_(includeSystem),
              allocator_(std::make_shared<LinearAllocator>(1024))
        {
            ASSERT(diagnosticSink)
            ASSERT(includeSystem)

            // Add builtin macros
            {
                const char* const builtinNames[] = { "__FILE__", "__LINE__" };
                const MacroDefinition::Opcode builtinOpcodes[] = { MacroDefinition::Opcode::BuiltinFile, MacroDefinition::Opcode::BuiltinLine };

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
                    case Token::Type::Invalid:
                        break;
                }
            }
        }

        // Find the currently-defined macro of the given name, or return NULL
        std::shared_ptr<MacroDefinition> PreprocessorImpl::LookupMacro(const U8String& name) const
        {
            const auto& search = macrosDefinitions_.find(name);
            return search != macrosDefinitions_.end() ? search->second : nullptr;
        }

        Token readToken();

        int32_t PreprocessorImpl::tokenToInt(const Token& token, int radix)
        {
            ASSERT(token.type == Token::Type::IntegerLiteral)

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

        uint32_t PreprocessorImpl::tokenToUInt(const Token& token, int radix)
        {
            ASSERT(token.type == Token::Type::IntegerLiteral)

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

        Token PreprocessorImpl::readToken()
        {
            for (;;)
            {
                auto inputFile = currentInputFile_;

                if (!inputFile)
                    return endOfFileToken_;

                auto expansionStream = currentInputFile_->GetExpansionStream();

                Token token = peekRawToken();
                if (token.type == Token::Type::EndOfFile)
                {
                    popInputFile();
                    continue;
                }

                // If we have a directive (`#` at start of line) then handle it
                if ((token.type == Token::Type::Pound) && IsSet(token.flags, Token::Flags::AtStartOfLine))
                {
                    // Parse and handle the directive
                    handleDirective();
                    continue;
                }
                /*

                // otherwise, if we are currently in a skipping mode, then skip tokens
                if (InputSource->isSkipping())
                {
                    expansionStream->readRawToken();
                    continue;
                }
                */

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

        void PreprocessorImpl::popInputFile()
        {
            const auto& inputFile = currentInputFile_;
            ASSERT(inputFile)

            // We expect the file to be at its end, so that the
            // next token read would be an end-of-file token.
            const auto& expansionStream = inputFile->GetExpansionStream();
            Token eofToken = expansionStream->PeekRawToken();
            ASSERT(eofToken.type == Token::Type::EndOfFile)

            // If there are any open preprocessor conditionals in the file, then
            // we need to diagnose them as an error, because they were not closed
            // at the end of the file. TODO
            /*
            for (auto conditional = inputFile->getInnerMostConditional(); conditional; conditional = conditional->parent)
            {
                GetSink(this)->diagnose(eofToken, Diagnostics::endOfFileInPreprocessorConditional);
                GetSink(this)->diagnose(conditional->ifToken, Diagnostics::seeDirective, conditional->ifToken.getContent());
            }
            */
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

        void PreprocessorImpl::setLexerDiagnosticSuppression(const std::shared_ptr<InputFile>& inputFile, bool shouldSuppressDiagnostics)
        {
            ASSERT(inputFile)

            if (shouldSuppressDiagnostics)
            {
                inputFile->GetLexer().EnableFlags(Lexer::Flags::SuppressDiagnostics);
            }
            else
            {
                inputFile->GetLexer().DisableFlags(Lexer::Flags::SuppressDiagnostics);
            }
        }

        bool PreprocessorImpl::expect(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken)
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

        bool PreprocessorImpl::expectRaw(DirectiveContext& context, Token::Type expected, DiagnosticInfo const& diagnostic, Token& outToken)
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
                {
                    sink_->Diagnose(context.token, Diagnostics::unexpectedTokensAfterDirective, getDirectiveName(context));
                }
                skipToEndOfLine();
            }
        }

        void PreprocessorImpl::handleDirective()
        {
            ASSERT(peekRawTokenType() == Token::Type::Pound)

            // Skip the `#`
            advanceRawToken();

            // Create a context for parsing the directive
            DirectiveContext context;

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
                sink_->Diagnose(context.token, Diagnostics::expectedPreprocessorDirectiveName);
                skipToEndOfLine();
                return;
            }

            // Look up the handler for the directive.
            const auto directive = findDirective(getDirectiveName(context));

            if (!IsSet(directive.flags, Directive::Flags::DontConsumeDirectiveAutomatically))
            {
                // Consume the directive name token.
                advanceRawToken();
            }

            // Call the directive-specific handler
            (this->*directive.function)(context);
        }

        void PreprocessorImpl::handleInvalidDirective(DirectiveContext& directiveContext)
        {
            sink_->Diagnose(directiveContext.token, Diagnostics::unknownPreprocessorDirective, getDirectiveName(directiveContext));
            skipToEndOfLine();
        }

        // Handle a `#define` directive
        void PreprocessorImpl::handleDefineDirective(DirectiveContext& directiveContext)
        {
            Token nameToken;
            if (!expectRaw(directiveContext, Token::Type::Identifier, Diagnostics::expectedTokenInPreprocessorDirective, nameToken))
                return;

            U8String name = nameToken.GetContentString();

            const auto& oldMacro = LookupMacro(name);
            if (oldMacro)
            {
                if (oldMacro->IsBuiltin())
                {
                    sink_->Diagnose(nameToken, Diagnostics::builtinMacroRedefinition, name);
                }
                else
                {
                    sink_->Diagnose(nameToken, Diagnostics::macroRedefinition, name);

                    if (oldMacro->GetNameToken().isValid())
                        sink_->Diagnose(oldMacro->GetNameToken(), Diagnostics::seePreviousDefinitionOf, name);
                }
            }
            auto macro = std::make_shared<MacroDefinition>();
            std::unordered_map<U8String, uint32_t> mapParamNameToIndex;

            // If macro name is immediately followed (with no space) by `(`,
            // then we have a function-like macro
            auto maybeOpenParen = peekRawToken();
            if (maybeOpenParen.type == Token::Type::LParent && !IsSet(maybeOpenParen.flags, Token::Flags::AfterWhitespace))
            {
                ASSERT_MSG(false, "NOT IMPLEMENTED");
                /* )
            {
                // This is a function-like macro, so we need to remember that
                // and start capturing parameters
                macro->flavor = MacroDefinition::Flavor::FunctionLike;

                AdvanceRawToken(context);

                // If there are any parameters, parse them
                if (PeekRawTokenType(context) != Token::Type::RParent)
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
                        //
                        Token paramNameToken;
                        if (PeekRawTokenType(context) != Token::Type::Ellipsis)
                        {
                            if (!ExpectRaw(context, Token::Type::Identifier, Diagnostics::expectedTokenInMacroParameters, &paramNameToken))
                                break;
                        }

                        // Whether or not a name was seen, we allow an ellipsis
                        // to indicate a variadic macro parameter.
                        //
                        // Note: a variadic parameter, if any, should always be
                        // the last parameter of a macro, but we do not enforce
                        // that requirement here.
                        //
                        Token ellipsisToken;
                        MacroDefinition::Param param;
                        if (PeekRawTokenType(context) == Token::Type::Ellipsis)
                        {
                            ellipsisToken = AdvanceRawToken(context);
                            param.isVariadic = true;
                        }

                        if (paramNameToken.type != Token::Type::Unknown)
                        {
                            // If we read an explicit name for the parameter, then we can use
                            // that name directly.
                            //
                            param.nameLoc.name = paramNameToken.getName();
                            param.nameLoc.loc = paramNameToken.loc;
                        }
                        else
                        {
                            // If an explicit name was not read for the parameter, we *must*
                            // have an unnamed variadic parameter. We know this because the
                            // only case where the logic above doesn't require a name to
                            // be read is when it already sees an ellipsis ahead.
                            //
                            SLANG_ASSERT(ellipsisToken.type != Token::Type::Unknown);

                            // Any unnamed variadic parameter is treated as one named `__VA_ARGS__`
                            //
                            param.nameLoc.name = context->m_preprocessor->getNamePool()->getName("__VA_ARGS__");
                            param.nameLoc.loc = ellipsisToken.loc;
                        }

                        // TODO(tfoley): The C standard seems to disallow certain identifiers
                        // (e.g., `defined` and `__VA_ARGS__`) from being used as the names
                        // of user-defined macros or macro parameters. This choice seemingly
                        // supports implementation flexibility in how the special meanings of
                        // those identifiers are handled.
                        //
                        // We could consider issuing diagnostics for cases where a macro or parameter
                        // uses such names, or we could simply provide guarantees about what those
                        // names *do* in the context of the Slang preprocessor.

                        // Add the parameter to the macro being deifned
                        auto paramIndex = macro->params.getCount();
                        macro->params.add(param);

                        auto paramName = param.nameLoc.name;
                        if (mapParamNameToIndex.ContainsKey(paramName))
                        {
                            GetSink(context)->diagnose(param.nameLoc.loc, Diagnostics::duplicateMacroParameterName, name);
                        }
                        else
                        {
                            mapParamNameToIndex[paramName] = paramIndex;
                        }

                        // If we see `)` then we are done with arguments
                        if (PeekRawTokenType(context) == Token::Type::RParent)
                            break;

                        ExpectRaw(context, Token::Type::Comma, Diagnostics::expectedTokenInMacroParameters);
                    }
                }

                ExpectRaw(context, Token::Type::RParent, Diagnostics::expectedTokenInMacroParameters);

                // Once we have parsed the macro parameters, we can perform the additional validation
                // step of checking that any parameters before the last parameter are not variadic.
                //
                Index lastParamIndex = macro->params.getCount() - 1;
                for (Index i = 0; i < lastParamIndex; ++i)
                {
                    auto& param = macro->params[i];
                    if (!param.isVariadic)
                        continue;

                    GetSink(context)->diagnose(param.nameLoc.loc, Diagnostics::variadicMacroParameterMustBeLast, param.nameLoc.name);

                    // As a precaution, we will unmark the variadic-ness of the parameter, so that
                    // logic downstream from this step doesn't have to deal with the possibility
                    // of a variadic parameter in the middle of the parameter list.
                    //
                    param.isVariadic = false;
                }*/
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
                        //
                        token.type = Token::Type::EndOfFile;
                        macro->tokens.push_back(token);
                        break;
                }
                break;
            }

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
                sink_->Diagnose(nameToken, Diagnostics::macroNotDefined, name);
                return;
            }

            // name was defined, so remove it
            macrosDefinitions_.erase(name);
        }

        U8String PreprocessorImpl::readDirectiveMessage()
        {
            U8String result;

            while (!isEndOfLine())
            {
                Token token = advanceRawToken();

                if (IsSet(token.flags, Token::Flags::AfterWhitespace))
                {
                    if (!result.empty())
                        result.append(" ");
                }

                result.append(token.stringSlice.Begin(), token.stringSlice.End());
            }

            return result;
        }

        // Handle a `#warning` directive
        void PreprocessorImpl::handleWarningDirective(DirectiveContext& directiveContext)
        {
            setLexerDiagnosticSuppression(currentInputFile_, true);

            // Consume the directive
            advanceRawToken();

            // Read the message.
            U8String message = readDirectiveMessage();

            setLexerDiagnosticSuppression(currentInputFile_, false);

            // Report the custom error.
            sink_->Diagnose(directiveContext.token, Diagnostics::userDefinedWarning, message);
        }

        // Handle a `#include` directive
        void PreprocessorImpl::handleIncludeDirective(DirectiveContext& directiveContext)
        {
            Token pathToken;
            if (!expect(directiveContext, Token::Type::StringLiteral, Diagnostics::expectedTokenInPreprocessorDirective, pathToken))
                return;

            U8String path = getFileNameTokenValue(pathToken);

            const auto& sourceView = directiveContext.token.sourceLocation.GetSourceView();
            const auto& includedFromPathInfo = sourceView->GetSourceFile();

            (void)includedFromPathInfo;

            // Find the path relative to the foundPath
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
            std::ignore = inputStream;
            ASSERT_MSG(false, "NOT IMPLEMENTED")
            //PushInputStream(inputStream);
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

            const auto& nextToken = peekRawToken();
            // Start new source view from end of new line sequence.
            const auto sourceLocation = nextToken.sourceLocation + nextToken.stringSlice.GetLength();

            HumaneSourceLocation humaneLocation(line, 1);
            const auto& sourceView = SourceView::CreateSplited(sourceLocation, humaneLocation, pathInfo);
            const auto& inputStream = std::make_shared<LexerInputStream>(sourceView, sink_);

            std::ignore = inputStream;
            // Drop current stream
            ASSERT_MSG(false, "NOT IMPLEMENTD")
            //    currentInputStream_->ForceClose();
            // popInputStream();
            //  PushInputStream(inputStream);
        }

        void PreprocessorImpl::parseMacroOps(std::shared_ptr<MacroDefinition>& macro,
                                             const std::unordered_map<U8String, uint32_t>& mapParamNameToIndex)
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
                    }
                    break;

                    case Token::Type::Pound:
                    {
                        auto paramNameTokenIndex = cursor;
                        auto paramNameToken = macro->tokens[paramNameTokenIndex];
                        if (paramNameToken.type != Token::Type::Identifier)
                        {
                            sink_->Diagnose(token, Diagnostics::expectedMacroParameterAfterStringize);
                            continue;
                        }

                        auto paramName = paramNameToken.GetContentString();

                        const auto search = mapParamNameToIndex.find(paramName);
                        if (search == mapParamNameToIndex.end())
                        {
                            sink_->Diagnose(token, Diagnostics::expectedMacroParameterAfterStringize);
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
                            sink_->Diagnose(token, Diagnostics::tokenPasteAtStart);
                            continue;
                        }

                        if (macro->tokens[cursor].type == Token::Type::EndOfFile)
                        {
                            sink_->Diagnose(token, Diagnostics::tokenPasteAtEnd);
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
                                   const std::shared_ptr<DiagnosticSink>& diagnosticSink)
            : impl_(std::make_unique<PreprocessorImpl>(includeSystem, diagnosticSink))
        {
        }

        void Preprocessor::PushInputFile(const std::shared_ptr<SourceFile>& sourceFile)
        {
            const auto sourceView = RR::Rfx::SourceView::Create(sourceFile);
            impl_->PushInputFile(std::make_shared<PreprocessorImpl::InputFile>(impl_, sourceView));
        }

        std::vector<Token> Preprocessor::ReadAllTokens()
        {
            ASSERT(impl_);
            return impl_->ReadAllTokens();
        }

        MacroInvocation::MacroInvocation(
            const std::shared_ptr<PreprocessorImpl>& preprocessor,
            const std::shared_ptr<DiagnosticSink>& sink,
            const std::shared_ptr<MacroDefinition>& macro,
            const SourceLocation& macroInvocationLoc,
            const Token& initiatingMacroToken)
            : preprocessor_(preprocessor),
              sink_(sink),
              m_macro(macro),
              m_macroInvocationLoc(macroInvocationLoc),
              initiatingMacroToken_(initiatingMacroToken)
        {
            m_firstBusyMacroInvocation = this;
        }

        void MacroInvocation::Prime(MacroInvocation* nextBusyMacroInvocation)
        {
            m_nextBusyMacroInvocation = nextBusyMacroInvocation;

            initCurrentOpStream();
            lookaheadToken_ = readTokenImpl();
        }

        Token MacroInvocation::readTokenImpl()
        {
            if (isClosed_)
                return lookaheadToken_;

            // The `MacroInvocation` type maintains an invariant that after each
            // call to `readTokenImpl`:
            //
            // * The `m_currentOpStreams` stack will be non-empty
            //
            // * The input state in `m_currentOpStreams` will correspond to the
            //   macro definition op at index `m_macroOpIndex`
            //
            // * The next token read from `m_currentOpStreams` will not be an EOF
            //   *unless* the expansion has reached the end of the macro invocaiton
            //
            // The first time `readTokenImpl()` is called, it will only be able
            // to rely on the weaker invariant guaranteed by `initCurrentOpStream()`:
            //
            // * The `m_currentOpStreams` stack will be non-empty
            //
            // * The input state in `m_currentOpStreams` will correspond to the
            //   macro definition op at index `m_macroOpIndex`
            //
            // * The next token read from `m_currentOpStreams` may be an EOF if
            //   the current op has an empty expansion.
            //
            // In either of those cases, we can start by reading the next token
            // from the expansion of the current op.
            Token token = m_currentOpStreams.ReadToken();
            auto tokenOpIndex = m_macroOpIndex;

            // Clone flags of token that "initiated" macro if we are at the beginning
            if (tokenOpIndex == 0)
                token.flags = initiatingMacroToken_.flags;

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
                if (m_currentOpStreams.PeekTokenType() != Token::Type::EndOfFile)
                {
                    // We know that we have tokens remaining to read from
                    // `m_currentOpStreams`, and we thus expect that the
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
                auto currentOpIndex = m_macroOpIndex;
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
                if (nextOpIndex == m_macro->ops.size())
                    return token;

                // Because `m_currentOpStreams` is at its end, we can pop all of
                // those streams to reclaim their memory before we push any new
                // ones.
                m_currentOpStreams.PopAll();

                // Now we've commited to moving to the next op in the macro
                // definition, and we want to push appropriate streams onto
                // the stack of input streams to represent that op.
                m_macroOpIndex = nextOpIndex;
                auto const& nextOp = m_macro->ops[nextOpIndex];

                // What we do depends on what the next op's opcode is.
                switch (nextOp.opcode)
                {
                    default:
                    {
                        // All of the easy cases are handled by `initCurrentOpStream()`
                        // which also gets invoked in the logic of `MacroInvocation::Prime()`
                        // to handle the first op in the definition.
                        //
                        // This operation will set up `m_currentOpStreams` so that it
                        // accurately reflects the expansion of the op at index `m_macroOpIndex`.
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
                            token = m_currentOpStreams.ReadToken();
                            tokenOpIndex = m_macroOpIndex;
                        }
                    }
                    break;

                    case MacroDefinition::Opcode::TokenPaste:
                    {
                        // The more complicated case is a token paste (`##`).
                        auto tokenPasteTokenIndex = nextOp.index0;
                        const auto& tokenPasteLoc = m_macro->tokens[tokenPasteTokenIndex].sourceLocation;
                        const auto& tokenPasteHumaneLoc = m_macro->tokens[tokenPasteTokenIndex].humaneSourceLocation;

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
                        // We could implement matching behavior in Slang with special-case logic here, but
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
                        // For Slang it may be simplest to solve this problem at the parser level, by allowing
                        // trailing commas in argument lists without error/warning. However, if we *do* decide
                        // to implement the gcc extension for `##` it would be logical to try to detect and
                        // intercept that special case here.

                        // If the `tokenOpIndex` that `token` was read from is the op right
                        // before the `##`, then we know it is the last token produced by
                        // the preceding op (or possibly an EOF if that op's expansion was empty).
                        if (tokenOpIndex == nextOpIndex - 1)
                        {
                            if (token.type != Token::Type::EndOfFile)
                                pastedContent << token.GetContentString();
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
                        // op follows the `##`. We will thus start by initialiing the `m_currentOpStrems`
                        // for reading from that op.
                        m_macroOpIndex++;
                        initCurrentOpStream();

                        // If the right operand yields at least one non-EOF token, then we need
                        // to append that content to our paste result.
                        Token rightToken = m_currentOpStreams.ReadToken();
                        if (rightToken.type != Token::Type::EndOfFile)
                            pastedContent << rightToken.GetContentString();

                        // Now we need to re-lex the token(s) that resulted from pasting, which requires
                        // us to create a fresh source file to represent the paste result.
                        PathInfo pathInfo = PathInfo::makeTokenPaste();

                        const auto& sourceFile = preprocessor_->GetIncludeSystem()->CreateFileFromString(pathInfo, pastedContent.str());
                        auto sourceView = SourceView::Create(sourceFile);

                        Lexer lexer(sourceView, preprocessor_->GetSink());
                        const auto& lexedTokens = lexer.LexAllSemanticTokens();

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
                            sink_->Diagnose(tokenPasteLoc, tokenPasteHumaneLoc, Diagnostics::invalidTokenPasteResult, pastedContent.str());

                        // No matter what sequence of tokens we got, we can create an input stream to represent
                        // them and push it as the representation of the `##` macro definition op.
                        //
                        // Note: the stream(s) created for the right operand will be on the stack under the new
                        // one we push for the pasted tokens, and as such the input state is capable of reading
                        // from both the input stream for the `##` through to the input for the right-hand-side
                        // op, which is consistent with `m_macroOpIndex`.
                        const auto& inputStream = std::make_shared<SingleUseInputStream>(lexedTokens);
                        m_currentOpStreams.Push(inputStream);

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
                            token = m_currentOpStreams.ReadToken();
                            tokenOpIndex = m_macroOpIndex;
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
            // The job of this function is to make sure that `m_currentOpStreams` is set up
            // to refelct the state of the op at `m_macroOpIndex`.
            auto opIndex = m_macroOpIndex;
            auto& op = m_macro->ops[opIndex];

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
                    auto tokenBuffer = m_macro->tokens.begin();
                    auto tokenReader = TokenReader(tokenBuffer + beginTokenIndex, tokenBuffer + endTokenIndex);
                    const auto& stream = std::make_shared<PretokenizedInputStream>(tokenReader);
                    m_currentOpStreams.Push(stream);
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
                    //
                    auto tokenReader = getArgTokens(paramIndex);

                    // Because expansion doesn't apply to this parameter reference, we can simply
                    // play back those tokens exactly as they appeared in the argument list.
                    const auto& stream = std::make_shared<PretokenizedInputStream>(tokenReader);
                    m_currentOpStreams.Push(stream);
                }
                break;

                case MacroDefinition::Opcode::ExpandedParam:
                {
                    // Most uses of a macro parameter will be subject to macro expansion.
                    //
                    // The initial logic here is similar to the unexpanded case above.
                    auto paramIndex = op.index1;
                    auto tokenReader = getArgTokens(paramIndex);
                    const auto& stream = std::make_shared<PretokenizedInputStream>(tokenReader);

                    // The only interesting addition to the unexpanded case is that we wrap
                    // the stream that "plays back" the argument tokens with a stream that
                    // applies macro expansion to them.
                    const auto& expansion = std::make_shared<ExpansionInputStream>(preprocessor_, stream);
                    m_currentOpStreams.Push(expansion);
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
                    const auto& loc = m_macro->tokens[tokenIndex].sourceLocation;

                    auto paramIndex = op.index1;
                    auto tokenReader = getArgTokens(paramIndex);

                    // A stringized parameter is always a `"`-enclosed string literal
                    // (there is no way to stringize things to form a character literal).
                    U8String string;
                    string.push_back('"');
                    for (bool first = true; !tokenReader.IsAtEnd(); first = false)
                    {
                        auto token = tokenReader.AdvanceToken();

                        // Any whitespace between the tokens of argument must be collapsed into
                        // a single space character. Fortunately for us, the lexer has tracked
                        // for each token whether it was immediately preceded by whitespace,
                        // so we can check for whitespace that precedes any token except the first.
                        if (!first && IsSet(token.flags, Token::Flags::AfterWhitespace))
                            string.push_back(' ');

                        // We need to rememember to apply escaping to the content of any tokens
                        // being pulled into the string. E.g., this would come up if we end up
                        // trying to stringize a literal like `"this"` because we need the resulting
                        // token to be `"\"this\""` which includes the quote characters in the string
                        // literal value.
                        StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, token.GetContentString(), string);
                    }
                    string.push_back('"');

                    // Once we've constructed the content of the stringized result, we need to push
                    // a new single-token stream that represents that content.
                    pushSingleTokenStream(Token::Type::StringLiteral, loc, string);
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
                    pushStreamForSourceLocBuiltin(Token::Type::IntegerLiteral, [=](U8String& string, const SourceLocation& loc, const HumaneSourceLocation& humaneLoc)
                                                  {
                                                      std::ignore = loc;
                                                      string += std::to_string(humaneLoc.line);
                                                  });
                }
                break;

                case MacroDefinition::Opcode::BuiltinFile:
                {
                    // The `__FILE__` case is quite similar to `__LINE__`, except for the type of token it yields,
                    // and the way it computes the desired token content.
                    pushStreamForSourceLocBuiltin(Token::Type::StringLiteral, [=](U8String& string, const SourceLocation& loc, const HumaneSourceLocation& humaneLoc)
                                                  {
                                                      std::ignore = humaneLoc;
                                                      StringEscapeUtil::AppendQuoted(StringEscapeUtil::Style::Cpp, loc.GetSourceView()->GetPathInfo().foundPath, string);
                                                  });
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
            for (auto busyMacroInvocation = duringMacroInvocation; busyMacroInvocation; busyMacroInvocation = busyMacroInvocation->m_nextBusyMacroInvocation)
            {
                if (busyMacroInvocation->m_macro == macro)
                    return true;
            }
            return false;
        }

        void MacroInvocation::pushSingleTokenStream(Token::Type tokenType, const SourceLocation& tokenLoc, U8String const& content)
        {
            // The goal here is to push a token stream that represents a single token
            // with exactly the given `content`, etc.
            // TODO COMMNET
            // We are going to keep the content alive using the slice pool for the source
            // manager, which will also lead to it being shared if used multiple times.
            const auto& allocator = preprocessor_->GetAllocator();
            const auto allocated = (char*)allocator->Allocate(content.length());
            std::copy(content.begin(), content.end(), allocated);

            Token token;
            token.type = tokenType;
            token.stringSlice = UnownedStringSlice(allocated, allocated + content.length());
            token.sourceLocation = tokenLoc;

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

            const auto& inputStream = std::make_shared<SingleUseInputStream>(/* m_preprocessor,*/ lexedTokens);
            m_currentOpStreams.Push(inputStream);
        }

        template <typename F>
        void MacroInvocation::pushStreamForSourceLocBuiltin(Token::Type tokenType, F const& valueBuilder)
        {
            // The `__LINE__` and `__FILE__` macros will always expand based on
            // the "initiating" source location, which should come from the
            // top-level file instead of any nested macros being expanded.
            const auto initiatingLoc = initiatingMacroToken_.sourceLocation;
            const auto humaneInitiatingLoc = initiatingMacroToken_.humaneSourceLocation;
            if (!initiatingLoc.IsValid())
            {
                // If we cannot find a valid source location for the initiating
                // location, then we will not expand the macro.
                //
                // TODO: Maybe we should issue a diagnostic here?
                ASSERT(false);
                return;
            }

            // The `valueBuilder` provided by the caller will determine what the content
            // of the token will be based on the source location (either to generate the
            // `__LINE__` or the `__FILE__` value).
            U8String content;
            valueBuilder(content, initiatingLoc, humaneInitiatingLoc);

            // Next we constuct and push an input stream with exactly the token type and content we want.
            pushSingleTokenStream(tokenType, m_macroInvocationLoc, content);
        }

        // Check whether the current token on the given input stream should be
        // treated as a macro invocation, and if so set up state for expanding
        // that macro.
        void ExpansionInputStream::maybeBeginMacroInvocation()
        {
            auto preprocessor = preprocessor_.lock();
            ASSERT(preprocessor)

            // We iterate because the first token in the expansion of one
            // macro may be another macro invocation.
            for (;;)
            {
                // TODO comment
                // The "next" token to be read is already in our `m_lookeadToken`
                // member, so we can simply inspect it.
                //
                // We also care about where that token came from (which input stream).
                Token token = PeekRawToken();

                // If the token is not an identifier, then it can't possibly name a macro.
                if (token.type != Token::Type::Identifier)
                    return;

                // We will look for a defined macro matching the name.
                //
                // If there isn't one this couldn't possibly be the start of a macro
                // invocation.
                const auto& name = token.GetContentString();
                const auto& macro = preprocessor->LookupMacro(name);

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
                // Note: it is critical here that `m_inputStreams.getTopStream()`
                // returns the top-most stream that was active when `m_lookaheadToken`
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
                        const auto& invocation = std::make_shared<MacroInvocation>(preprocessor,
                                                                                   preprocessor->GetSink(),
                                                                                   macro,
                                                                                   token.sourceLocation,
                                                                                   initiatingMacroToken_);

                        // Consume initializatin macro
                        readTokenImpl();

                        invocation->Prime(busyMacros);
                        pushMacroInvocation(invocation);
                    }
                    break;

                    case MacroDefinition::Flavor::FunctionLike:
                    {
                        ASSERT_MSG(false, "NOT IMPLEMENTED");
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
                        // While we don't expect users to make heavy use of this feature in Slang,
                        // it is worthwhile to try to stay compatible.
                        //
                        // Because the macro name is already in `m_lookaheadToken`, we can peak
                        // at the underlying input stream to see if the next non-whitespace
                        // token after the lookahead is a `(`.
                        /*m_inputStreams.skipAllWhitespace();
                        Token maybeLeftParen = m_inputStreams.peekToken();
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
                            //
                            return;
                        }

                        // If we saw an opening `(`, then we know we are starting some kind of
                        // macro invocation, although we don't yet know if it is well-formed.
                        //
                        MacroInvocation* invocation = new MacroInvocation(preprocessor, macro, token.loc, initiatingMacroInvocationLoc_);

                        // We start by consuming the opening `(` that we checked for above.
                        //
                        Token leftParen = m_inputStreams.readToken();
                        SLANG_ASSERT(leftParen.type == Token::Type::LParent);

                        // Next we parse any arguments to the macro invocation, which will
                        // consist of `()`-balanced sequences of tokens separated by `,`s.
                        //
                        _parseMacroArgs(macro, invocation);
                        Index argCount = invocation->getArgCount();

                        // We expect th arguments to be followed by a `)` to match the opening
                        // `(`, and if we don't find one we need to diagnose the issue.
                        //
                        if (m_inputStreams.peekTokenType() == Token::Type::RParent)
                        {
                            m_inputStreams.readToken();
                        }
                        else
                        {
                            GetSink(preprocessor)->diagnose(m_inputStreams.peekLoc(), Diagnostics::expectedTokenInMacroArguments, Token::Type::RParent, m_inputStreams.peekTokenType());
                        }

                        // The number of arguments at the macro invocation site might not
                        // match the number of arguments declared for the macro. In this
                        // case we diagnose an issue *and* skip expansion of this invocation
                        // (it effectively expands to zero new tokens).
                        const Index paramCount = Index(macro->params.getCount());
                        if (!macro->isVariadic())
                        {
                            // The non-variadic case is simple enough: either the argument
                            // count exactly matches the required parameter count, or we
                            // diagnose an error.
                            //
                            if (argCount != paramCount)
                            {
                                GetSink(preprocessor)->diagnose(leftParen.loc, Diagnostics::wrongNumberOfArgumentsToMacro, paramCount, argCount);
                                return;
                            }
                        }
                        else
                        {
                            // In the variadic case, we only require arguments for the
                            // non-variadic parameters (all but the last one). In addition,
                            // we do not consider it an error to have more than the required
                            // number of arguments.
                            //
                            Index requiredArgCount = paramCount - 1;
                            if (argCount < requiredArgCount)
                            {
                                GetSink(preprocessor)->diagnose(leftParen.loc, Diagnostics::wrongNumberOfArgumentsToMacro, requiredArgCount, argCount);
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
                        //
                        auto nextStream = m_inputStreams.getNextStream();
                        auto busyMacrosForFunctionLikeInvocation = nextStream->getFirstBusyMacroInvocation();

                        // Consume initializatin macro
                        readTokenImpl();

                        invocation->Prime(busyMacrosForFunctionLikeInvocation);
                        pushMacroInvocation(invocation);*/
                    }
                    break;
                }
            }
        }

        TokenReader MacroInvocation::getArgTokens(uint32_t paramIndex)
        {
            ASSERT(paramIndex >= 0);
            ASSERT(paramIndex < m_macro->params.size());

            // How we determine the range of argument tokens for a parameter
            // depends on whether or not it is a variadic parameter.
            auto& param = m_macro->params[paramIndex];
            const auto argTokens = m_argTokens.begin();
            if (!param.isVariadic)
            {
                // The non-variadic case is, as expected, the simpler one.
                //
                // We expect that there must be an argument at the index corresponding
                // to the parameter, and we construct a `TokenReader` that will play
                // back the tokens of that argument.
                ASSERT(paramIndex < m_args.size());
                auto arg = m_args[paramIndex];

                // TODO outofrange possible
                return TokenReader(argTokens + arg.beginTokenIndex, argTokens + arg.endTokenIndex);
            }
            else
            {
                ASSERT(m_args.size() > 0);
                // In the variadic case, it is possible that we have zero or more
                // arguments that will all need to be played back in any place where
                // the variadic parameter is referenced.
                //
                // The first relevant argument is the one at the index coresponding
                // to the variadic parameter, if any. The last relevant argument is
                // the last argument to the invocation, *if* there was a first
                // relevant argument.
                auto firstArgIndex = paramIndex;
                auto lastArgIndex = m_args.size() - 1;

                // One special case is when there are *no* arguments coresponding
                // to the variadic parameter.
                if (firstArgIndex > lastArgIndex)
                {
                    // When there are no arguments for the varaidic parameter we will
                    // construct an empty token range that comes after the other arguments.
                    auto arg = m_args[lastArgIndex];
                    return TokenReader(argTokens + arg.endTokenIndex, argTokens + arg.endTokenIndex);
                }

                // Because the `m_argTokens` array includes the commas between arguments,
                // we can get the token sequence we want simply by making a reader that spans
                // all the tokens between the first and last argument (inclusive) that correspond
                // to the variadic parameter.
                auto firstArg = m_args[firstArgIndex];
                auto lastArg = m_args[lastArgIndex];
                return TokenReader(argTokens + firstArg.beginTokenIndex, argTokens + lastArg.endTokenIndex);
            }
        }

        void ExpansionInputStream::pushMacroInvocation(const std::shared_ptr<MacroInvocation>& expansion)
        {
            inputStreams_.Push(expansion);
        }
    }
}