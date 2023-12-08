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

    namespace
    {
        enum class Precedence : int32_t
        {
            Invalid = -1,
            Null = 0,
            TernaryConditional,
            LogicalOr,
            LogicalAnd,
            BitOr,
            BitXor,
            BitAnd,
            EqualityComparison,
            RelationalComparison,
            BitShift,
            Additive,
            Multiplicative,
            Prefix,
            Postfix,
        };

        Precedence getInfixOpPrecedence(const Token& opToken)
        {
            switch (opToken.type)
            {
                case Token::Type::QuestionMark: return Precedence::TernaryConditional;
                case Token::Type::OpOr: return Precedence::LogicalOr;
                case Token::Type::OpAnd: return Precedence::LogicalAnd;
                case Token::Type::OpBitOr: return Precedence::BitOr;
                case Token::Type::OpBitXor: return Precedence::BitXor;
                case Token::Type::OpBitAnd: return Precedence::BitAnd;
                case Token::Type::OpEql:
                case Token::Type::OpNeq: return Precedence::EqualityComparison;
                case Token::Type::OpGreater:
                case Token::Type::OpGeq:
                case Token::Type::OpLeq:
                case Token::Type::OpLess: return Precedence::RelationalComparison;
                case Token::Type::OpRsh:
                case Token::Type::OpLsh: return Precedence::BitShift;
                case Token::Type::OpAdd:
                case Token::Type::OpSub: return Precedence::Additive;
                case Token::Type::OpMul:
                case Token::Type::OpDiv:
                case Token::Type::OpMod: return Precedence::Multiplicative;
                default: return Precedence::Invalid;
            }
        }

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
        RSONValue parseAndEvaluateInfixExpressionWithPrecedence(RSONValue left, Precedence precedence);
        RSONValue evaluateInfixOp(const Token& opToken, const RSONValue& left, const RSONValue& right);

        RSONValue parseArray();
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
        // First read in the left-hand side (or the whole expression in the unary case)
        const auto value = parseAndEvaluateUnaryExpression();

        // Try to read in trailing infix operators with correct precedence
        return parseAndEvaluateInfixExpressionWithPrecedence(value, Precedence::Null);
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
            case Token::Type::LBracket: return parseArray();
            case Token::Type::LBrace: return parseObject();
            default:
                // TODO syntax error
                getSink().Diagnose(peekToken(), Diagnostics::syntaxErrorInPreprocessorExpression);
                return {};
        }
    }

    // Parse the rest of an infix preprocessor expression with
    // precedence greater than or equal to the given `precedence` argument.
    // The value of the left-hand-side expression is provided as
    // an argument.
    // This is used to form a simple recursive-descent expression parser.
    RSONValue ParserImpl::parseAndEvaluateInfixExpressionWithPrecedence(RSONValue left, Precedence precedence)
    {
        for (;;)
        {
            // Look at the next token, and see if it is an operator of
            // high enough precedence to be included in our expression
            const auto& opToken = peekToken();
            const auto opPrecedence = getInfixOpPrecedence(opToken);

            // If it isn't an operator of high enough precedence, we are done.
            if (opPrecedence < precedence)
                break;

            // Otherwise we need to consume the operator token.
            advance();

            // Next we parse a right-hand-side expression by starting with
            // a unary expression and absorbing and many infix operators
            // as possible with strictly higher precedence than the operator
            // we found above.
            auto right = parseAndEvaluateUnaryExpression();
            for (;;)
            {
                // Look for an operator token
                Token rightOpToken = peekToken();
                const auto rightOpPrecedence = getInfixOpPrecedence(rightOpToken);

                // If no operator was found, or the operator wasn't high
                // enough precedence to fold into the right-hand-side,
                // exit this loop.
                if (rightOpPrecedence <= opPrecedence)
                    break;

                // Now invoke the parser recursively, passing in our
                // existing right-hand side to form an even larger one.
                right = parseAndEvaluateInfixExpressionWithPrecedence(right, rightOpPrecedence);
            }

            // Now combine the left- and right-hand sides using
            // the operator we found above.
            left = evaluateInfixOp(opToken, left, right);
        }
        return left;
    }
    /*
    bool isCompareOperation(const Token& opToken)
    {
        return (opToken.type == Token::Type::OpLess) ||
               (opToken.type == Token::Type::OpGreater) ||
               (opToken.type == Token::Type::OpLeq) ||
               (opToken.type == Token::Type::OpGeq) ||
               (opToken.type == Token::Type::OpEql) ||
               (opToken.type == Token::Type::OpNeq);
    }*/

    template <typename T>
    RSONValue compareOps(const Token& opToken, T left, T right)
    {
        switch (opToken.type)
        {
            case Token::Type::OpLess: return RSONValue::MakeBool(left < right);
            case Token::Type::OpGreater: return RSONValue::MakeBool(left > right);
            case Token::Type::OpLeq: return RSONValue::MakeBool(left <= right);
            case Token::Type::OpGeq: return RSONValue::MakeBool(left >= right);
            case Token::Type::OpEql: return RSONValue::MakeBool(left == right);
            case Token::Type::OpNeq: return RSONValue::MakeBool(left != right);
        }

        return {};
    }

    template <typename Ret, typename LV, typename RV>
    RSONValue arithmeticOps(const Token& opToken, LV left, RV right)
    {
        static_assert(std::is_same<double, Ret>() || std::is_same<int64_t, Ret>(), "return type should be double or int64");

        Ret result = 0;
        switch (opToken.type)
        {
            case Token::Type::OpMul: result = left * right; break;
            case Token::Type::OpAdd: result = left + right; break;
            case Token::Type::OpSub: result = left - right; break;
            case Token::Type::OpDiv:
            {
                if (right == RV(0))
                {
                    //  if (!context.parseError)
                    //      GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);

                    return {};
                }
                result = left / right;
                break;
            }
            case Token::Type::OpMod:
            {
                if (std::is_same<double, Ret>())
                    return {};

                if (right == RV(0))
                {
                    //  if (!context.parseError)
                    //      GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);

                    return {};
                }
                result = Ret(int64_t(left) % int64_t(right));
                break;
            }
            default: return {};
        }

        return std::is_same<double, Ret>() ? RSONValue::MakeFloat(double(result)) : RSONValue::MakeInt(int64_t(result));
    }

    // Evaluate one infix operation in a preprocessor
    // conditional
    RSONValue ParserImpl::evaluateInfixOp(const Token& opToken, const RSONValue& left, const RSONValue& right)
    {
        if (opToken.type == Token::Type::QuestionMark)
        {
            //  if (!context.parseError)
            //      GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);
            return {};
        }

        if (left.type == RSONValue::Type::Integer || right.type == RSONValue::Type::Integer)
        {
            switch (opToken.type)
            {
                case Token::Type::OpLsh: return RSONValue::MakeInt(left.AsInteger() << right.AsInteger());
                case Token::Type::OpRsh: return RSONValue::MakeInt(left.AsInteger() >> right.AsInteger());
                case Token::Type::OpBitAnd: return RSONValue::MakeInt(left.AsInteger() & right.AsInteger());
                case Token::Type::OpBitOr: return RSONValue::MakeInt(left.AsInteger() | right.AsInteger());
                case Token::Type::OpBitXor: return RSONValue::MakeInt(left.AsInteger() ^ right.AsInteger());
            }
        }

        if (left.type == RSONValue::Type::Bool || right.type == RSONValue::Type::Bool)
        {
            switch (opToken.type)
            {
                case Token::Type::OpAnd: return RSONValue::MakeBool(left.AsBool() && right.AsBool());
                case Token::Type::OpOr: return RSONValue::MakeBool(left.AsBool() || right.AsBool());
            }
        }

        if ((left.type == RSONValue::Type::Integer || left.type == RSONValue::Type::Float) &&
            (right.type == RSONValue::Type::Integer || right.type == RSONValue::Type::Float))
        {
            const RSONValue::Type return_type = (left.type == RSONValue::Type::Integer && right.type == RSONValue::Type::Integer)
                                                    ? RSONValue::Type::Integer
                                                    : RSONValue::Type::Float;

            switch (return_type)
            {
                case RSONValue::Type::Float:
                {
                    auto result = compareOps<double>(opToken, left.AsFloat(), right.AsFloat());
                    return result.IsValid() ? result : arithmeticOps<double>(opToken, left.AsFloat(), right.AsFloat());
                }
                case RSONValue::Type::Integer:
                {
                    auto result = compareOps<int64_t>(opToken, left.AsInteger(), right.AsInteger());
                    return result.IsValid() ? result : arithmeticOps<int64_t>(opToken, left.AsInteger(), right.AsInteger());
                }
            }
        }
        //  if (!context.parseError)
        //      GetSink().Diagnose(opToken, Diagnostics::divideByZeroInPreprocessorExpression);

        return {};
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

            RSONValue value = parseAndEvaluateExpression();
            if (value.type == RSONValue::Type::Invalid)
                return value;

            RR_RETURN_VALUE_ON_FAIL(builder_.AddValue(std::move(value)), RSONValue {});

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
        skipAllWhitespaces();

        while (true)
        {
            skipAllWhitespaces();

            if (peekTokenType() == Token::Type::RBrace ||
                peekTokenType() == Token::Type::EndOfFile)
                break;

            Token keyToken;

            RR_RETURN_ON_FAIL(expect({ Token::Type::Identifier, Token::Type::StringLiteral, Token::Type::OpLsh }, keyToken));
            if (keyToken.type != Token::Type::OpLsh)
                RR_RETURN_ON_FAIL(expect(Token::Type::Colon));

            skipAllWhitespaces();

            RSONValue value = parseAndEvaluateExpression();
            if (value.type == RSONValue::Type::Invalid)
                return RResult::Fail;

            if (keyToken.type == Token::Type::OpLsh)
            {
                RR_RETURN_ON_FAIL(builder_.Inheritance(keyToken, value));
            }
            else
            {
                RR_RETURN_ON_FAIL(builder_.AddKeyValue(keyToken, value));
            }

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