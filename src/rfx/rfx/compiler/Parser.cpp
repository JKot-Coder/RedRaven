#include "Parser.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/JSONBuilder.hpp"
#include "rfx/compiler/Lexer.hpp"

#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"

namespace RR::Rfx
{
    using RResult = Common::RResult;

    /// An token reader that reads tokens directly using the `Lexer`
    struct LexerReader
    {
    public:
        LexerReader() = delete;

        LexerReader(const std::shared_ptr<SourceView>& sourceView,
                    const std::shared_ptr<CompileContext>& context)
        {
            lexer_ = std::make_unique<Lexer>(sourceView, context);
            lookaheadToken_ = readTokenImpl();
        }

        Lexer& GetLexer() const { return *lexer_; }

        // A common thread to many of the input stream implementations is to
        // use a single token of lookahead in order to suppor the `peekToken()`
        // operation with both simplicity and efficiency.
        Token ReadToken()
        {
            auto result = lookaheadToken_;
            lookaheadToken_ = readTokenImpl();
            return result;
        }

        Token PeekToken() const { return lookaheadToken_; }
        Token::Type PeekTokenType() const { return lookaheadToken_.type; }

    private:
        /// Read a token from the lexer, bypassing lookahead
        Token readTokenImpl()
        {
            for (;;)
            {
                Token token = lexer_->ReadToken();
                switch (token.type)
                {
                    case Token::Type::WhiteSpace:
                    case Token::Type::BlockComment:
                    case Token::Type::LineComment:
                    case Token::Type::InvalidCharacter:
                        continue;

                    default: return Token(token);
                }
            }
        }

    private:
        /// The lexer state that will provide input
        std::unique_ptr<Lexer> lexer_;

        /// One token of lookahead
        Token lookaheadToken_;
    };

    class ParserImpl
    {
    public:
        ParserImpl() = delete;
        ParserImpl(const std::shared_ptr<SourceView>& sourceView,
                   const std::shared_ptr<CompileContext>& context)
            : context_(context),
              builder_(context),
              lexerReader_(sourceView, context)
        {
            ASSERT(context);
        }

        RResult Parse();

    private:
        DiagnosticSink& getSink() const { return context_->sink; }

        Token peekToken() const { return lexerReader_.PeekToken(); }
        Token::Type peekTokenType() const { return lexerReader_.PeekTokenType(); }
        Token advance() { return lexerReader_.ReadToken(); }

        void skipAllWhitespaces()
        {
            // All other whitespaces skipped by reader
            while (peekTokenType() == Token::Type::NewLine)
                advance();
        }

        bool advanceIf(std::initializer_list<Token::Type> tokens, Token& outToken);
        bool inline advanceIf(Token::Type tokenType, Token& outToken) { return advanceIf({ tokenType }, outToken); }
        bool inline advanceIf(Token::Type tokenType)
        {
            Token token;
            return advanceIf(tokenType, token);
        }

        RResult parseArray();
        RResult parseDeclaration();
        RResult parseObject(bool renameMe = false);
        RResult parseValue();
        RResult parseStringValue();

        RResult expect(std::initializer_list<Token::Type> tokens, Token& outToken);
        RResult expect(Token::Type expected, Token& outToken) { return expect({ expected }, outToken); };
        RResult expect(Token::Type expected)
        {
            Token token;
            return expect(expected, token);
        };

    private:
        std::shared_ptr<CompileContext> context_;
        JSONBuilder builder_;
        LexerReader lexerReader_;
    };

    bool ParserImpl::advanceIf(std::initializer_list<Token::Type> tokens, Token& outToken)
    {
        const auto lookAheadTokenType = peekTokenType();

        for (const auto& token : tokens)
            if (lookAheadTokenType == token)
            {
                outToken = advance();
                return true;
            }

        return false;
    }

    RResult ParserImpl::parseArray()
    {
        RR_RETURN_ON_FAIL(expect(Token::Type::LBracket));
        RR_RETURN_ON_FAIL(builder_.StartArray(peekToken()));

        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBracket)
                break;

            RR_RETURN_ON_FAIL(parseValue());

            if (peekTokenType() == Token::Type::Comma ||
                peekTokenType() == Token::Type::NewLine)
            {
                advance();
                continue;
            }

            break;
        }

        RR_RETURN_ON_FAIL(expect(Token::Type::RBracket));
        builder_.EndArray();

        return RResult::Ok;
    }

    RResult ParserImpl::parseDeclaration()
    {
        Token token;
        RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, token));
        RR_RETURN_ON_FAIL(builder_.AddKey(token));

        if (peekTokenType() == Token::Type::OpLess)
        {
            builder_.BeginInrehitance();
            advance();

            while (true)
            {
                RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, token));
                RR_RETURN_ON_FAIL(builder_.AddParent(token));

                if (peekTokenType() == Token::Type::Comma ||
                    peekTokenType() == Token::Type::NewLine)
                {
                    advance();
                    continue;
                }

                break;
            }
        }

        return RResult::Ok;
    }

    RResult ParserImpl::parseObject(bool renameMe)
    {
        if (!renameMe)
        {
            RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));
            RR_RETURN_ON_FAIL(builder_.StartObject(peekToken()));
        }

        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBrace ||
                peekTokenType() == Token::Type::EndOfFile)
                break;

            RR_RETURN_ON_FAIL(parseDeclaration());
            RR_RETURN_ON_FAIL(expect(Token::Type::Colon));

            skipAllWhitespaces();

            RR_RETURN_ON_FAIL(parseValue());

            if (peekTokenType() == Token::Type::Comma ||
                peekTokenType() == Token::Type::NewLine)
            {
                advance();
                continue;
            }

            break;
        }

        if (!renameMe)
        {
            RR_RETURN_ON_FAIL(expect(Token::Type::RBrace));
            builder_.EndObject();
        }
        else
        {
            RR_RETURN_ON_FAIL(expect(Token::Type::EndOfFile));
        }

        return RResult::Ok;
    }

    RResult ParserImpl::parseValue()
    {
        switch (peekTokenType())
        {
            case Token::Type::StringLiteral:
            case Token::Type::CharLiteral:
            case Token::Type::Identifier:
            case Token::Type::IntegerLiteral:
            case Token::Type::FloatingPointLiteral:
            {
                const auto& token = advance();
                return builder_.AddValue(token);
            }
            case Token::Type::LBracket: return parseArray();
            case Token::Type::LBrace: return parseObject();
            default:
            {
                getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                return RResult::Fail;
            }
        }
    }

    RResult ParserImpl::expect(std::initializer_list<Token::Type> tokens, Token& out)
    {
        const auto lookAheadTokenType = peekTokenType();

        for (const auto& token : tokens)
            if (token == lookAheadTokenType)
            {
                out = advance();
                return RResult::Ok;
            }

        const U8String& tokensString = fmt::format("{}\n", fmt::join(tokens, " or "));
        getSink().Diagnose(peekToken(), Diagnostics::unexpectedTokenExpectedTokenType, peekTokenType(), tokensString);
        return RResult::Fail;
    }

    RResult ParserImpl::Parse()
    {
        RR_RETURN_ON_FAIL(parseObject(true));
        return expect(Token::Type::EndOfFile);
    }

    Parser::~Parser() { }

    Parser::Parser(const std::shared_ptr<SourceView>& sourceView,
                   const std::shared_ptr<CompileContext>& context)
        : impl_(std::make_unique<ParserImpl>(sourceView, context))
    {
    }

    RResult Parser::Parse()
    {
        ASSERT(impl_)
        return impl_->Parse();
    }
}
