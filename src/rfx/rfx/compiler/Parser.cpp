#include "Parser.hpp"

#include "compiler/AST.hpp"
#include "compiler/ASTBuilder.hpp"
#include "compiler/DiagnosticCore.hpp"
#include "compiler/Lexer.hpp"

#include "common/OnScopeExit.hpp"

namespace RR::Rfx
{
    struct DeclBase
    {
    };

    struct SyntaxDecl : DeclBase
    {
    };

    struct Decl : DeclBase
    {
        /* data */
    };

    enum class MatchedTokenType : uint32_t
    {
        Parentheses,
        SquareBrackets,
        CurlyBraces,
        File,
    };

    /// Information on how to parse certain pairs of matches tokens
    struct MatchedTokenInfo
    {
        /// The token type that opens the pair
        Token::Type openTokenType;

        /// The token type that closes the pair
        Token::Type closeTokenType;

        /// A list of token types that should lead the parser
        /// to abandon its search for a matchign closing token
        /// (terminated by `Token::Type::EndOfFile`).
        const Token::Type* bailAtCloseTokens;
    };

    static const Token::Type BailAtEOF[] = { Token::Type::EndOfFile };
    static const Token::Type BailAtCurlyBraceOrEOF[] = { Token::Type::RBrace, Token::Type::EndOfFile };
    static const MatchedTokenInfo MatchedTokenInfos[] = {
        { Token::Type::LParent, Token::Type::RParent, BailAtCurlyBraceOrEOF },
        { Token::Type::LBracket, Token::Type::RBracket, BailAtCurlyBraceOrEOF },
        { Token::Type::LBrace, Token::Type::RBrace, BailAtEOF },
        { Token::Type::Unknown, Token::Type::EndOfFile, BailAtEOF },
    };

    class ParserImpl
    {
    public:
        ParserImpl() = delete;
        ParserImpl(const std::shared_ptr<DiagnosticSink>& diagnosticSink)
            : sink_(diagnosticSink)
        {
            ASSERT(diagnosticSink);
        }

        void Parse(const std::vector<Token>& tokens,
                   const std::shared_ptr<ASTBuilder>& astBuilder);

    private:
        void pushScope();
        void popScope();

        bool lookAheadToken(Token::Type type) const;

        Token peekToken() const;
        Token::Type peekTokenType() const;
        bool advanceIf(Token::Type tokenType, Token& outToken);
        bool advanceIfMatch(MatchedTokenType matchedTokenType, Token& outToken);

        Token readToken(Token::Type expected, bool forceSkippingToClosingToken);
        Token readToken(Token::Type expected) { return readToken(expected, false); };

        DeclBase* parseDecl();
        DeclBase* parseDeclaratorDecl();
        DeclBase* parseDeclWithModifiers();

        template <typename T>
        bool tryParseUsingSyntaxDecl(T*& outSyntax);

        SyntaxDecl* tryLookUpSyntaxDecl(const U8String& name);

    private:
        std::shared_ptr<DiagnosticSink> sink_;
        Scope* currentScope_;
        std::shared_ptr<ASTBuilder> astBuilder_;
        TokenReader tokenReader_;
    };

    Token ParserImpl::peekToken() const
    {
        return tokenReader_.PeekToken();
    }

    Token::Type ParserImpl::peekTokenType() const
    {
        return tokenReader_.PeekTokenType();
    }

    template <typename T>
    bool ParserImpl::tryParseUsingSyntaxDecl(T*& outSyntax)
    {
        if (peekTokenType() != Token::Type::Identifier)
            return false;

        const auto nameToken = peekToken();
        const auto name = nameToken.GetContentString();

        auto syntaxDecl = tryLookUpSyntaxDecl(name);

        if (!syntaxDecl)
            return false;

        return tryParseUsingSyntaxDecl(syntaxDecl, outSyntax);
    }

    SyntaxDecl* ParserImpl::tryLookUpSyntaxDecl(const U8String& name)
    {
        std::ignore = name;
        // Let's look up the name and see what we find.
        /* auto lookupResult = lookUp(
            astBuilder_,
            nullptr, // no semantics visitor available yet
            name,
            currentScope_);*/

        // If we didn't find anything, or the result was overloaded,
        // then we aren't going to be able to extract a single decl.
        // if (!lookupResult.isValid() || lookupResult.isOverloaded())
        return nullptr;
        /*
                auto decl = lookupResult.item.declRef.getDecl();
                auto syntaxDecl = as<SyntaxDecl>(decl);

                return syntaxDecl;*/
    }

    void ParserImpl::pushScope()
    {
        auto* newScope = astBuilder_->Create<Scope>();
        newScope->parent = currentScope_;
        currentScope_ = newScope;
    }

    void ParserImpl::popScope()
    {
        ASSERT(currentScope_);
        currentScope_ = currentScope_->parent;
    }

    bool ParserImpl::lookAheadToken(Token::Type type) const
    {
        return peekTokenType() == type;
    }

    bool ParserImpl::advanceIf(Token::Type tokenType, Token& outToken)
    {
        if (lookAheadToken(tokenType))
        {
            outToken = tokenReader_.AdvanceToken();
            return true;
        }
        return false;
    }

    DeclBase* ParserImpl::parseDecl(/*ContainerDecl*  containerDecl*/)
    {
        // Modifiers modifiers = ParseModifiers(parser);
        return parseDeclWithModifiers(/*containerDecl, modifiers*/);
    }

    DeclBase* ParserImpl::parseDeclaratorDecl()
    {
        const auto startToken = peekToken();

        // Parse a type name
        auto typeName = readToken(TokenType::Identifier);

        // We may need to build up multiple declarations in a group,
        // but the common case will be when we have just a single
        // declaration
        DeclGroupBuilder declGroupBuilder;
        declGroupBuilder.startToken = startToken;
        declGroupBuilder.astBuilder = astBuilder_;

        // The type specifier may include a declaration. E.g.,
        // it might declare a `struct` type.
        if (typeSpec.decl)
            declGroupBuilder.addDecl(typeSpec.decl);

        if (advanceIf(Token::Type::Semicolon))
        {
            // No actual variable is being declared here, but
            // that might not be an error.

            auto result = declGroupBuilder.getResult();
            if (!result)
                sink_->Diagnose(startToken, Diagnostics::declarationDidntDeclareAnything);
            return result;
        }

        // It is possible that we have a plain `struct`, `enum`,
        // or similar declaration that isn't being used to declare
        // any variable, and the user didn't put a trailing
        // semicolon on it:
        //
        //      struct Batman
        //      {
        //          int cape;
        //      }
        //
        // We want to allow this syntax (rather than give an
        // inscrutable error), but also support the less common
        // idiom where that declaration is used as part of
        // a variable declaration:
        //
        //      struct Robin
        //      {
        //          float tights;
        //      } boyWonder;

        InitDeclarator initDeclarator = parseInitDeclarator(kDeclaratorParseOptions_None);

        DeclaratorInfo declaratorInfo;
        declaratorInfo.typeSpec = typeSpec.expr;

        // Rather than parse function declarators properly for now,
        // we'll just do a quick disambiguation here. This won't
        // matter unless we actually decide to support function-type parameters,
        // using C syntax.
        //
        if ((peekTokenType() == Token::Type::LParent ||
             peekTokenType() == Token::Type::OpLess)

            // Only parse as a function if we didn't already see mutually-exclusive
            // constructs when parsing the declarator.
            && !initDeclarator.initializer && !initDeclarator.semantics.first)
        {
            // Looks like a function, so parse it like one.
            UnwrapDeclarator(parser->astBuilder, initDeclarator, &declaratorInfo);
            return parseTraditionalFuncDecl(parser, declaratorInfo);
        }

        // Otherwise we are looking at a variable declaration, which could be one in a sequence...

        if (AdvanceIf(parser, TokenType::Semicolon))
        {
            // easy case: we only had a single declaration!
            UnwrapDeclarator(parser->astBuilder, initDeclarator, &declaratorInfo);
            VarDeclBase* firstDecl = CreateVarDeclForContext(parser->astBuilder, containerDecl);
            CompleteVarDecl(parser, firstDecl, declaratorInfo);

            declGroupBuilder.addDecl(firstDecl);
            return declGroupBuilder.getResult();
        }

        // Otherwise we have multiple declarations in a sequence, and these
        // declarations need to somehow share both the type spec and modifiers.
        //
        // If there are any errors in the type specifier, we only want to hear
        // about it once, so we need to share structure rather than just
        // clone syntax.

        auto sharedTypeSpec = astBuilder_->Create<SharedTypeExpr>();
        sharedTypeSpec->loc = typeSpec.expr->loc;
        sharedTypeSpec->base = TypeExp(typeSpec.expr);

        for (;;)
        {
            declaratorInfo.typeSpec = sharedTypeSpec;
            UnwrapDeclarator(astBuilder_, initDeclarator, &declaratorInfo);

            VarDeclBase* varDecl = CreateVarDeclForContext(astBuilder_, containerDecl);
            CompleteVarDecl(parser, varDecl, declaratorInfo);

            declGroupBuilder.addDecl(varDecl);

            // end of the sequence?
            if (AdvanceIf(parser, TokenType::Semicolon))
                return declGroupBuilder.getResult();

            // ad-hoc recovery, to avoid infinite loops
            if (parser->isRecovering)
            {
                parser->ReadToken(TokenType::Semicolon);
                return declGroupBuilder.getResult();
            }

            // Let's default to assuming that a missing `,`
            // indicates the end of a declaration,
            // where a `;` would be expected, and not
            // a continuation of this declaration, where
            // a `,` would be expected (this is tailoring
            // the diagnostic message a bit).
            //
            // TODO: a more advanced heuristic here might
            // look at whether the next token is on the
            // same line, to predict whether `,` or `;`
            // would be more likely...

            if (!advanceIf(Token::Type::Comma))
            {
                readToken(Token::Type::Semicolon);
                return declGroupBuilder.getResult();
            }

            // expect another variable declaration...
            initDeclarator = parseInitDeclarator(parser, kDeclaratorParseOptions_None);
        }
    }

    DeclBase* ParserImpl::parseDeclWithModifiers()
    {
        DeclBase* decl = nullptr;

        //  const auto loc = tokenReader_.PeekLoc();

        switch (peekTokenType())
        {
            case Token::Type::Identifier:
            {
                // A declaration that starts with an identifier might be:
                //
                // - A keyword-based declaration (e.g., `cbuffer ...`)
                // - The beginning of a type in a declarator-based declaration (e.g., `int ...`)

                // First we will check whether we can use the identifier token
                // as a declaration keyword and parse a declaration using
                // its associated callback:
                Decl* parsedDecl = nullptr;
                if (tryParseUsingSyntaxDecl<Decl>(parsedDecl))
                {
                    decl = parsedDecl;
                    break;
                }

                // Our final fallback case is to assume that the user is
                // probably writing a C-style declarator-based declaration.
                decl = parseDeclaratorDecl(/*containerDecl, modifiers*/);
                break;
            }

                // If nothing else matched, we try to parse an "ordinary" declarator-based declaration
            default:
                decl = parseDeclaratorDecl(/*containerDecl, modifiers*/);
                break;
        }

        return decl;
    }

    bool ParserImpl::advanceIfMatch(MatchedTokenType matchedTokenType, Token& outToken)
    {
        // The behavior of the seatch for a match can depend on the
        // type of matches tokens we are parsing.
        const auto& matchedTokenInfo = MatchedTokenInfos[uint32_t(matchedTokenType)];

        if (advanceIf(matchedTokenInfo.closeTokenType, outToken))
            return true;

        return false;
    }

    Token readToken(TokenType expected, bool forceSkippingToClosingToken);
    {
        if (tokenReader.peekTokenType() == expected)
        {
            isRecovering = false;
            sameTokenPeekedTimes = 0;
            return tokenReader.advanceToken();
        }

        if (!isRecovering)
        {
            Unexpected(this, expected);
            if (!forceSkippingToClosingToken)
                return tokenReader.peekToken();
            switch (expected)
            {
            case TokenType::RBrace:
            case TokenType::RParent:
            case TokenType::RBracket:
                break;
            default:
                return tokenReader.peekToken();
            }
        }

        // Try to find a place to recover
        if (TryRecoverBefore(this, expected))
        {
            isRecovering = false;
            return tokenReader.advanceToken();
        }
        // This could be dangerous: if `ReadToken()` is being called
        // in a loop we may never make forward progress, so we use
        // a counter to limit the maximum amount of times we are allowed
        // to peek the same token. If the outter parsing logic is
        // correct, we will pop back to the right level. If there are
        // erroneous parsing logic, this counter is to prevent us
        // looping infinitely.
        static const int kMaxTokenPeekCount = 64;
        sameTokenPeekedTimes++;
        if (sameTokenPeekedTimes < kMaxTokenPeekCount)
            return tokenReader.peekToken();
        else
        {
            sameTokenPeekedTimes = 0;
            return tokenReader.advanceToken();
        }
    }

    void ParserImpl::Parse(const std::vector<Token>& tokens,
                           const std::shared_ptr<ASTBuilder>& astBuilder)
    {
        ASSERT(astBuilder);
        ASSERT(!astBuilder_);

        astBuilder_ = astBuilder;
        ON_SCOPE_EXIT({ astBuilder_ = nullptr; });

        Token closingBraceToken;
        while (!advanceIfMatch(MatchedTokenType::File, closingBraceToken))
        {
            parseDecl(/*containerDecl*/);
        }
        // containerDecl->closingSourceLoc = closingBraceToken.loc;

        std::ignore = tokens;
    }

    Parser::~Parser() { }

    Parser::Parser(const std::shared_ptr<DiagnosticSink>& diagnosticSink)
        : impl_(std::make_unique<ParserImpl>(diagnosticSink))
    {
    }

    void Parser::Parse(const std::vector<Token>& tokens,
                       const std::shared_ptr<ASTBuilder>& astBuilder)
    {
        ASSERT(impl_);
        impl_->Parse(tokens, astBuilder);
    }
}
