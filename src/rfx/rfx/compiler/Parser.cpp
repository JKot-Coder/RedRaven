#include "Parser.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/Lexer.hpp"
#include "rfx/compiler/RSONBuilder.hpp"

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

    class ParserImpl final : Common::NonCopyable
    {
    public:
        ParserImpl() = delete;
        ParserImpl(const TokenSpan& tokenSpan,
                   const std::shared_ptr<CompileContext>& context)
            : context_(context),
              builder_(context),
              tokenSpan_(tokenSpan),
              currentToken_(tokenSpan_.begin())
        {
            ASSERT(context);
        }

        RResult Parse(RSONValue& root);

    private:
        DiagnosticSink& getSink() const { return context_->sink; }

        Token peekToken() const { return *currentToken_; }
        Token::Type peekTokenType() const { return currentToken_->type; }
        Token advance()
        {
            auto currentToken = *currentToken_;
            auto currentIter = currentToken_;

            if (++currentIter < tokenSpan_.end())
                currentToken_ = currentIter;

            return currentToken;
        }

        void skipAllWhitespaces()
        {
            // All other whitespaces skipped by reader
            while (peekTokenType() == Token::Type::NewLine)
                advance();
        }

        bool advanceIf(std::initializer_list<Token::Type> expected, Token& outToken);
        bool inline advanceIf(Token::Type expected, Token& outToken) { return advanceIf({ expected }, outToken); }
        bool inline advanceIf(Token::Type expected)
        {
            Token token;
            return advanceIf(expected, token);
        }

        RResult tokenToInt(const Token& token, int radix, int64_t& outValue);
        RResult tokenToFloat(const Token& token, double& outValue);

        RSONValue parseAndEvaluateExpression();
        RSONValue parseAndEvaluateUnaryExpression();

        RSONValue parseArray();
        RResult tryParseParents();
        RResult parseParents();
        RSONValue parseObject();
        RSONValue parseRoot();
        RResult parseObjectBody();
        RSONValue parseNumber();
        RSONValue parseString();

        RResult expect(std::initializer_list<Token::Type> expected, Token& outToken);
        RResult expect(Token::Type expected, Token& outToken) { return expect({ expected }, outToken); };
        RResult expect(Token::Type expected)
        {
            Token token;
            return expect(expected, token);
        };

    private:
        std::shared_ptr<CompileContext> context_;
        RSONBuilder builder_;
        TokenSpan tokenSpan_;
        TokenSpan::const_iterator currentToken_;
    };

    namespace
    {
        enum class UnaryOps
        {
            Sub,
            Add,
            Not,
            BitNot
        };

        RSONValue applyUnaryOp(UnaryOps op, const RSONValue& value)
        {
            switch (value.type)
            {
                case RSONValue::Type::Bool:
                    switch (op)
                    {
                        case UnaryOps::Sub: return {};
                        case UnaryOps::Add: return {};
                        case UnaryOps::Not: return RSONValue::MakeBool(!value.AsBool());
                        case UnaryOps::BitNot: return {};
                    }
                    break;
                case RSONValue::Type::Integer:
                    switch (op)
                    {
                        case UnaryOps::Add: return value;
                        case UnaryOps::Sub: return RSONValue::MakeInt(-value.AsInteger());
                        case UnaryOps::Not: return {};
                        case UnaryOps::BitNot: return RSONValue::MakeInt(~value.AsInteger());
                    }
                    break;
                case RSONValue::Type::Float:
                    switch (op)
                    {
                        case UnaryOps::Add: return value;
                        case UnaryOps::Sub: return RSONValue::MakeFloat(-value.AsFloat());
                        case UnaryOps::Not: return {};
                        case UnaryOps::BitNot: return {};
                    }
                    break;
                default: return {};
            }

            ASSERT_MSG(false, "Unknown unaryOp");
            return {};
        }
    }

    RResult ParserImpl::tokenToInt(const Token& token, int radix, int64_t& outValue)
    {
        ASSERT(token.type == Token::Type::IntegerLiteral);
        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.End());
        outValue = std::strtol(token.stringSlice.Begin(), &end, radix);

        if (errno == ERANGE)
        {
            getSink().Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "int64_t");
            return RResult::Fail;
        }

        if (end != token.stringSlice.End())
        {
            getSink().Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), radix);
            return RResult::Fail;
        }

        return RResult::Ok;
    }

    RResult ParserImpl::tokenToFloat(const Token& token, double& outValue)
    {
        ASSERT(token.type == Token::Type::FloatingPointLiteral);
        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.End());
        outValue = std::strtod(token.stringSlice.Begin(), &end);

        if (errno == ERANGE)
        {
            getSink().Diagnose(token, Diagnostics::floatLiteralOutOfRange, token.GetContentString(), "double");
            return RResult::Fail;
        }

        if (end != token.stringSlice.End())
        {
            getSink().Diagnose(token, Diagnostics::floatLiteralUnexpected, token.GetContentString());
            return RResult::Fail;
        }

        return RResult::Ok;
    }

    bool ParserImpl::advanceIf(std::initializer_list<Token::Type> expected, Token& outToken)
    {
        const auto lookAheadTokenType = peekTokenType();

        for (const auto& expectedType : expected)
            if (lookAheadTokenType == expectedType)
            {
                outToken = advance();
                return true;
            }

        return false;
    }

    /// Parse a complete (infix) preprocessor expression, and return its value
    RSONValue ParserImpl::parseAndEvaluateExpression()
    {
        RR_RETURN_VALUE_ON_FAIL(tryParseParents(), RSONValue {});

        // First read in the left-hand side (or the whole expression in the unary case)
        const auto value = parseAndEvaluateUnaryExpression();

        // Try to read in trailing infix operators with correct precedence
        return value;
        // parseAndEvaluateInfixExpressionWithPrecedence(context, value, 0);
    }

    // Parse a unary (prefix) expression inside of a preprocessor directive.
    RSONValue ParserImpl::parseAndEvaluateUnaryExpression()
    {
        switch (peekTokenType())
        {
            case Token::Type::EndOfFile:
            case Token::Type::NewLine:
                //    GetSink().Diagnose(peekToken(), Diagnostics::syntaxErrorInPreprocessorExpression);
                return {};
            default: break;
        }

        switch (peekTokenType())
        {
            // handle prefix unary ops
            case Token::Type::OpAdd: advance(); return applyUnaryOp(UnaryOps::Add, parseAndEvaluateUnaryExpression());
            case Token::Type::OpSub: advance(); return applyUnaryOp(UnaryOps::Sub, parseAndEvaluateUnaryExpression());
            case Token::Type::OpNot: advance(); return applyUnaryOp(UnaryOps::Not, parseAndEvaluateUnaryExpression());
            case Token::Type::OpBitNot: advance(); return applyUnaryOp(UnaryOps::BitNot, parseAndEvaluateUnaryExpression());

            // handle parenthized sub-expression
            case Token::Type::LParent:
            {
                Token leftParen = advance();
                const auto value = parseAndEvaluateExpression();

                if (RR_FAILED(expect(Token::Type::RParent)))
                {
                    getSink().Diagnose(leftParen, Diagnostics::seeOpeningToken, leftParen.stringSlice);
                    return {};
                }

                return value;
            }
            case Token::Type::IntegerLiteral:
            case Token::Type::FloatingPointLiteral:
                return parseNumber();
                /* case Token::Type::Identifier:
                {
                    // An identifier here means it was not defined as a macro (or
                    // it is defined, but as a function-like macro. These should
                    // just evaluate to zero (possibly with a warning)
                    getSink().Diagnose(token, Diagnostics::undefinedIdentifierInPreprocessorExpression, token.stringSlice);
                    return {};
                }*/

            case Token::Type::StringLiteral:
            case Token::Type::CharLiteral:
            case Token::Type::Identifier:
                return parseString();
                // case Token::Type::OpAdd:
            case Token::Type::LBracket: return parseArray();
            case Token::Type::LBrace: return parseObject();
            default:
                getSink().Diagnose(peekToken(), Diagnostics::syntaxErrorInPreprocessorExpression);
                return {};
        }
    }

    RSONValue ParserImpl::parseArray()
    {
        ASSERT(advance().type == Token::Type::LBracket);
        RR_RETURN_VALUE_ON_FAIL(builder_.StartArray(), RSONValue {});

        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBracket)
                break;

            auto qwe = peekToken();

            RSONValue value = parseAndEvaluateExpression();
            if (value.type == RSONValue::Type::Invalid)
                return value;

            RR_RETURN_VALUE_ON_FAIL(builder_.AddValue(qwe, std::move(value)), RSONValue {});

            if (peekTokenType() == Token::Type::Comma ||
                peekTokenType() == Token::Type::NewLine)
            {
                advance();
                continue;
            }

            break;
        }

        RR_RETURN_VALUE_ON_FAIL(expect(Token::Type::RBracket), RSONValue {});
        return builder_.EndArray();
    }
    /*
    RResult ParserImpl::parseArray()
    {
        RR_RETURN_ON_FAIL(expect(Token::Type::LBracket));
        RR_RETURN_ON_FAIL(builder_.StartArray());

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
    }*/

    RResult ParserImpl::tryParseParents()
    {
        Token token;

        if (peekTokenType() != Token::Type::OpLess)
            return RResult::Ok;

        advance();
        builder_.StartInrehitance();

        while (true)
        {
            RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, token));
            RR_RETURN_ON_FAIL(builder_.AddParent(token));

            if (peekTokenType() == Token::Type::Comma)
            {
                advance();
                continue;
            }

            break;
        }

        RR_RETURN_ON_FAIL(expect(Token::Type::OpGreater, token));
        builder_.EndInrehitance();
        return RResult::Ok;
    }

    RResult ParserImpl::parseParents()
    {
        Token token;

        RR_RETURN_ON_FAIL(expect({ Token::Type::OpLess, Token::Type::StringLiteral }, token));
        builder_.StartInrehitance();

        while (true)
        {
            RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, token));
            RR_RETURN_ON_FAIL(builder_.AddParent(token));

            if (peekTokenType() == Token::Type::Comma)
            {
                advance();
                continue;
            }

            break;
        }

        RR_RETURN_ON_FAIL(expect({ Token::Type::OpGreater, Token::Type::StringLiteral }, token));
        builder_.EndInrehitance();
        return RResult::Ok;
    }

    RSONValue ParserImpl::parseObject()
    {
        ASSERT(RR_SUCCEEDED(expect(Token::Type::LBrace)));
        RR_RETURN_VALUE_ON_FAIL(builder_.StartObject(), RSONValue {});

        RR_RETURN_VALUE_ON_FAIL(parseObjectBody(), RSONValue {});

        RR_RETURN_VALUE_ON_FAIL(expect(Token::Type::RBrace), RSONValue {});
        return builder_.EndObject();
    }

    RSONValue ParserImpl::parseRoot()
    {
        RR_RETURN_VALUE_ON_FAIL(parseObjectBody(), RSONValue {});
        RR_RETURN_VALUE_ON_FAIL(expect(Token::Type::EndOfFile), RSONValue {});
        return builder_.GetRootValue();
    }

    RResult ParserImpl::parseObjectBody()
    {
        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBrace ||
                peekTokenType() == Token::Type::EndOfFile)
                break;

            Token keyToken;
            RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, keyToken));
            RR_RETURN_ON_FAIL(expect(Token::Type::Colon));

            skipAllWhitespaces();

            RSONValue value = parseAndEvaluateExpression();
            if (value.type == RSONValue::Type::Invalid)
                return RResult::Fail;

            RR_RETURN_ON_FAIL(builder_.AddKeyValue(keyToken, value));

            if (peekTokenType() == Token::Type::Comma ||
                peekTokenType() == Token::Type::NewLine)
            {
                advance();
                continue;
            }

            break;
        }
        return RResult::Ok;
    }

    /*
    // Todo rename
    RResult ParserImpl::parseObject(bool renameMe)
    {
        if (!renameMe)
        {
            RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));
            RR_RETURN_ON_FAIL(builder_.StartObject());
        }

        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBrace ||
                peekTokenType() == Token::Type::EndOfFile)
                break;

            Token keyToken;
            RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral }, keyToken));
            RR_RETURN_ON_FAIL(expect(Token::Type::Colon));

            skipAllWhitespaces();

            RSONValue value = parseAndEvaluateExpression();
            if(value.type == RSONValue::Type::Invalid)
                return RResult::Fail;

            RR_RETURN_ON_FAIL(builder_.AddKeyValue(keyToken, value));

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
    }*/

    RSONValue ParserImpl::parseNumber()
    {
        Token token = advance();
        switch (token.type)
        {
            case Token::Type::IntegerLiteral:
            {
                int64_t value;
                if (tokenToInt(token, 0, value) != RResult::Ok)
                    return {};

                return RSONValue::MakeInt(value);
            }
            case Token::Type::FloatingPointLiteral:
            {
                double value;
                if (tokenToFloat(token, value) != RResult::Ok)
                    return {};

                return RSONValue::MakeFloat(value);
            }
            default:
            {
                getSink().Diagnose(token, Diagnostics::unexpectedToken, token.type);
                return {};
            }
        }
    }
    /*
    RResult ParserImpl::parseNumber(bool positive)
    {
        switch (peekTokenType())
        {
            case Token::Type::IntegerLiteral:
            {
                int64_t value;
                RR_RETURN_ON_FAIL(tokenToInt(advance(), 0, value));
                return builder_.AddValue({} ,RSONValue::MakeInt(value * (positive ? 1 : -1)));
            }
            case Token::Type::FloatingPointLiteral:
            {
                double value;
                RR_RETURN_ON_FAIL(tokenToFloat(advance(), value));
                return builder_.AddValue({}, RSONValue::MakeFloat(value * (positive ? 1.0 : -1.0)));
            }
            default:
            {
                getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                return RResult::Fail;
            }
        }
    }*/

    RSONValue ParserImpl::parseString()
    {
        const UnownedStringSlice stringSlice = advance().stringSlice;

        if (stringSlice == "null")
            return RSONValue::MakeNull();
        else if (stringSlice == "true")
            return RSONValue::MakeBool(true);
        else if (stringSlice == "false")
            return RSONValue::MakeBool(false);

        return RSONValue::MakeString(stringSlice);
    }

    /*
    RResult ParserImpl::parseString()
    {
        const UnownedStringSlice stringSlice = advance().stringSlice;

        if (stringSlice == "null")
            return builder_.AddValue({}, RSONValue::MakeNull());
        else if (stringSlice == "true")
            return builder_.AddValue({}, RSONValue::MakeBool(true));
        else if (stringSlice == "false")
            return builder_.AddValue({}, RSONValue::MakeBool(false));

        return builder_.AddValue({}, RSONValue::MakeString(stringSlice));
    }*/

    /*
    RResult ParserImpl::parseValue()
    {
        if (peekTokenType() == Token::Type::OpLess)
            RR_RETURN_ON_FAIL(parseParents());

        switch (peekTokenType())
        {
            case Token::Type::StringLiteral:
            case Token::Type::CharLiteral:
            case Token::Type::Identifier:
                return parseString();
            case Token::Type::OpAdd:
            case Token::Type::OpSub:
                return parseNumber(advance().type == Token::Type::OpAdd);
            case Token::Type::IntegerLiteral:
            case Token::Type::FloatingPointLiteral:
                return parseNumber();
            case Token::Type::LBracket: return parseArray();
            case Token::Type::LBrace: return parseObject();
            default:
            {
                getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                return RResult::Fail;
            }
        }
    }*/

    RResult ParserImpl::expect(std::initializer_list<Token::Type> expected, Token& out)
    {
        const auto lookAheadTokenType = peekTokenType();

        for (const auto& expectedType : expected)
            if (expectedType == lookAheadTokenType)
            {
                out = advance();
                return RResult::Ok;
            }

        const U8String& tokensString = fmt::format("{}", fmt::join(expected, " or "));
        getSink().Diagnose(peekToken(), Diagnostics::unexpectedTokenExpectedTokenType, peekTokenType(), tokensString);
        return RResult::Fail;
    }

    RResult ParserImpl::Parse(RSONValue& root)
    {
        root = parseRoot();

        if (root.type == RSONValue::Type::Invalid)
            return RResult::Fail;

        RR_RETURN_ON_FAIL(expect(Token::Type::EndOfFile));
        return RResult::Ok;
    }

    Parser::~Parser() { }

    Parser::Parser(const TokenSpan& tokenSpan,
                   const std::shared_ptr<CompileContext>& context)
        : impl_(std::make_unique<ParserImpl>(tokenSpan, context))
    {
    }

    RResult Parser::Parse(RSONValue& root)
    {
        ASSERT(impl_)
        return impl_->Parse(root);
    }
}