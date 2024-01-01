#include "Parser.hpp"

#include "rfx/core/StringUtils.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/Lexer.hpp"
#include "rfx/compiler/RSONBuilder.hpp"

#include "common/EnumClassOperators.hpp"
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
            Multiplicative
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

            ASSERT_MSG(false, "Unreachable");
            return {};
        }

        RSONValue applyUnaryOp(DiagnosticSink& sink, const Token& opToken, const RSONValue& value)
        {
            switch (value.type)
            {
                case RSONValue::Type::Bool:
                    if (opToken.type == Token::Type::OpNot)
                        return RSONValue::MakeBool(!value.AsBool());
                    break;
                case RSONValue::Type::Integer:
                    switch (opToken.type)
                    {
                        case Token::Type::OpAdd: return value;
                        case Token::Type::OpSub: return RSONValue::MakeInt(-value.AsInteger());
                        case Token::Type::OpBitNot: return RSONValue::MakeInt(~value.AsInteger());
                    }
                    break;
                case RSONValue::Type::Float:
                    switch (opToken.type)
                    {
                        case Token::Type::OpAdd: return value;
                        case Token::Type::OpSub: return RSONValue::MakeFloat(-value.AsFloat());
                    }
                    break;
            }

            sink.Diagnose(opToken, Diagnostics::wrongTypeForUnary, opToken.type, value.type);
            return {};
        }

        template <typename Ret, typename T>
        RSONValue arithmeticOps(DiagnosticSink& sink, const Token& opToken, T left, T right)
        {
            static_assert(std::is_same<double, Ret>() || std::is_same<int64_t, Ret>(), "return type should be Double or Int64");

            Ret result = 0;
            switch (opToken.type)
            {
                case Token::Type::OpMul: result = left * right; break;
                case Token::Type::OpAdd: result = left + right; break;
                case Token::Type::OpSub: result = left - right; break;
                case Token::Type::OpDiv:
                {
                    if (right == T(0))
                    {
                        sink.Diagnose(opToken, Diagnostics::divideByZero);
                        return {};
                    }
                    result = left / right;
                    break;
                }
                case Token::Type::OpMod:
                {
                    if (std::is_same<double, Ret>())
                    {
                        sink.Diagnose(opToken, Diagnostics::inflixOnlyValidForType, opToken.type, "Integers");
                        return {};
                    }

                    if (right == T(0))
                    {
                        sink.Diagnose(opToken, Diagnostics::divideByZero);
                        return {};
                    }
                    result = Ret(int64_t(left) % int64_t(right));
                    break;
                }
                default: return {};
            }

            return std::is_same<double, Ret>() ? RSONValue::MakeFloat(double(result)) : RSONValue::MakeInt(int64_t(result));
        }

        enum class OperationType
        {
            Arithmetic,
            Bitwise,
            Boolean,
            Compare,
            Invalid
        };

        OperationType getOperationType(Token::Type tokenType)
        {
            switch (tokenType)
            {
                case RR::Rfx::Token::Type::OpAdd:
                case RR::Rfx::Token::Type::OpSub:
                case RR::Rfx::Token::Type::OpMul:
                case RR::Rfx::Token::Type::OpDiv:
                case RR::Rfx::Token::Type::OpMod:
                    return OperationType::Arithmetic;

                case Token::Type::OpLsh:
                case Token::Type::OpRsh:
                case Token::Type::OpBitAnd:
                case Token::Type::OpBitOr:
                case Token::Type::OpBitXor:
                    return OperationType::Bitwise;

                case Token::Type::OpLess:
                case Token::Type::OpGreater:
                case Token::Type::OpLeq:
                case Token::Type::OpGeq:
                case Token::Type::OpEql:
                case Token::Type::OpNeq:
                    return OperationType::Compare;

                case RR::Rfx::Token::Type::OpAnd:
                case RR::Rfx::Token::Type::OpOr:
                    return OperationType::Boolean;

                default: return OperationType::Invalid;
            }
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

        RResult tokenToInt(const Token& token, int64_t& outValue);
        RResult tokenToFloat(const Token& token, double& outValue);

        RSONValue parseAndEvaluateExpression(Precedence precedence = Precedence::Null);
        RSONValue parseAndEvaluateUnaryExpression();
        RSONValue parseAndEvaluateInfixExpressionWithPrecedence(RSONValue left, Precedence precedence);
        RSONValue evaluateInfixOp(const Token& opToken, const RSONValue& left, const RSONValue& right);

        RSONValue parseArray();
        RSONValue parseObject();
        RSONValue parseRoot();
        RResult parseObjectBody();
        RSONValue parseNumber();
        RSONValue parseIdentifier();

        RResult expect(std::initializer_list<Token::Type> expected, Token& outToken);
        RResult expect(Token::Type expected, Token& outToken) { return expect({ expected }, outToken); };
        RResult expect(Token::Type expected)
        {
            Token token;
            return expect(expected, token);
        };

    private:
        enum class Flags : uint16_t
        {
            None = 0,
            ParentExpressionParse = 1 << 0,
        };
        ENUM_CLASS_FRIEND_BITWISE_OPS(Flags)

    private:
        Flags flags_;
        std::shared_ptr<CompileContext> context_;
        RSONBuilder builder_;
        TokenSpan tokenSpan_;
        TokenSpan::const_iterator currentToken_;
    };

    RResult ParserImpl::tokenToInt(const Token& token, int64_t& outValue)
    {
        ASSERT(token.type == Token::Type::IntegerLiteral);

        const auto result = StringUtils::StringToInt64(token.stringSlice, outValue);

        if (result == RResult::ArithmeticOverflow)
        {
            getSink().Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "int64_t");
            return result;
        }

        if (result == RResult::InvalidArgument)
        {
            getSink().Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), 0);
            return RResult::InvalidArgument;
        }

        return result;
    }

    RResult ParserImpl::tokenToFloat(const Token& token, double& outValue)
    {
        ASSERT(token.type == Token::Type::FloatingPointLiteral);
        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.end());
        outValue = std::strtod(token.stringSlice.begin(), &end);

        if (errno == ERANGE)
        {
            getSink().Diagnose(token, Diagnostics::floatLiteralOutOfRange, token.GetContentString(), "double");
            return RResult::Fail;
        }

        if (end != token.stringSlice.end())
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

    // Parse a complete (infix) expression, and return its value
    RSONValue ParserImpl::parseAndEvaluateExpression(Precedence precedence)
    {
        // First read in the left-hand side (or the whole expression in the unary case)
        const auto value = parseAndEvaluateUnaryExpression();

        if (!value.IsValid())
            return value;

        // Try to read in trailing infix operators with correct precedence
        return parseAndEvaluateInfixExpressionWithPrecedence(value, precedence);
    }

    // Parse a unary (prefix) expression
    RSONValue ParserImpl::parseAndEvaluateUnaryExpression()
    {
        switch (peekTokenType())
        {
            // handle prefix unary ops
            case Token::Type::OpAdd:
            case Token::Type::OpSub:
            case Token::Type::OpNot:
            case Token::Type::OpBitNot:
            {
                Token token = advance();
                return applyUnaryOp(getSink(), token, parseAndEvaluateUnaryExpression());
            }

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
                return RSONValue::MakeString(advance().stringSlice);
            case Token::Type::Identifier:
                return parseIdentifier();
            case Token::Type::LBracket: return parseArray();
            case Token::Type::LBrace: return parseObject();
            default:
                getSink().Diagnose(peekToken(), Diagnostics::syntaxError);
                return {};
        }
    }

    // Parse the rest of an infix expression with
    // precedence greater than or equal to the given `precedence` argument.
    // The value of the left-hand-side expression is provided as
    // an argument.
    // This is used to form a simple recursive-descent expression parser.
    RSONValue ParserImpl::parseAndEvaluateInfixExpressionWithPrecedence(RSONValue left, Precedence precedence)
    {
        for (;;)
        {
            if (!left.IsValid())
                return left;

            // Look at the next token, and see if it is an operator of
            // high enough precedence to be included in our expression
            const auto& opToken = peekToken();
            const auto opPrecedence = getInfixOpPrecedence(opToken);

            // If it isn't an operator of high enough precedence, we are done.
            if (opPrecedence < precedence)
                break;

            // Otherwise we need to consume the operator token.
            advance();

            // Special case the `?:` operator since it is the
            // one non-binary case we need to deal with.
            if (opToken.type == Token::Type::QuestionMark)
            {
                auto trueValue = parseAndEvaluateExpression(opPrecedence);
                RR_RETURN_VALUE_ON_FAIL(expect(Token::Type::Colon), RSONValue {});
                auto falseValue = parseAndEvaluateExpression(opPrecedence);
                // Todo validate
                left = left.AsBool() ? trueValue : falseValue;
                continue;
            }

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

    // Evaluate one infix operation
    RSONValue ParserImpl::evaluateInfixOp(const Token& opToken, const RSONValue& left, const RSONValue& right)
    {
        const auto opType = getOperationType(opToken.type);

        switch (opType)
        {
            case OperationType::Bitwise:
            {
                if (left.type != RSONValue::Type::Integer || right.type != RSONValue::Type::Integer)
                {
                    getSink().Diagnose(opToken, Diagnostics::inflixOnlyValidForType, opToken.type, "Integers");
                    return {};
                }

                switch (opToken.type)
                {
                    case Token::Type::OpLsh: return RSONValue::MakeInt(left.AsInteger() << right.AsInteger());
                    case Token::Type::OpRsh: return RSONValue::MakeInt(left.AsInteger() >> right.AsInteger());
                    case Token::Type::OpBitAnd: return RSONValue::MakeInt(left.AsInteger() & right.AsInteger());
                    case Token::Type::OpBitOr: return RSONValue::MakeInt(left.AsInteger() | right.AsInteger());
                    case Token::Type::OpBitXor: return RSONValue::MakeInt(left.AsInteger() ^ right.AsInteger());
                }

                ASSERT_MSG(false, "Unreachable");
                return {};
            }

            case OperationType::Compare:
            case OperationType::Arithmetic:
            {
                const bool valid = ((left.type == RSONValue::Type::Integer || left.type == RSONValue::Type::Float) &&
                                    (right.type == RSONValue::Type::Integer || right.type == RSONValue::Type::Float));

                if (!valid)
                {
                    getSink().Diagnose(opToken, Diagnostics::inflixOnlyValidForType, opToken.type, "Integers or Floats");
                    return {};
                }

                const RSONValue::Type return_type = (left.type == RSONValue::Type::Integer && right.type == RSONValue::Type::Integer)
                                                        ? RSONValue::Type::Integer
                                                        : RSONValue::Type::Float;

                switch (return_type)
                {
                    case RSONValue::Type::Float:
                    {
                        return (opType == OperationType::Compare)
                                   ? compareOps<double>(opToken, left.AsFloat(), right.AsFloat())
                                   : arithmeticOps<double>(getSink(), opToken, left.AsFloat(), right.AsFloat());
                    }
                    case RSONValue::Type::Integer:
                    {
                        return (opType == OperationType::Compare)
                                   ? compareOps<int64_t>(opToken, left.AsInteger(), right.AsInteger())
                                   : arithmeticOps<int64_t>(getSink(), opToken, left.AsInteger(), right.AsInteger());
                    }
                }

                ASSERT_MSG(false, "Unreachable");
                return {};
            }

            case OperationType::Boolean:
            {
                if (left.type != RSONValue::Type::Bool || right.type != RSONValue::Type::Bool)
                {
                    getSink().Diagnose(opToken, Diagnostics::inflixOnlyValidForType, opToken.type, "Bool");
                    return {};
                }

                switch (opToken.type)
                {
                    case Token::Type::OpAnd: return RSONValue::MakeBool(left.AsBool() && right.AsBool());
                    case Token::Type::OpOr: return RSONValue::MakeBool(left.AsBool() || right.AsBool());
                }

                ASSERT_MSG(false, "Unreachable");
                return {};
            }

            default: return {};
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

            if (keyToken.type == Token::Type::OpLsh)
            {
                flags_ = Flags::ParentExpressionParse;
                RSONValue value = parseAndEvaluateExpression();
                if (value.type == RSONValue::Type::Invalid)
                    return RResult::Fail;

                flags_ = Flags::None;
                RR_RETURN_ON_FAIL(builder_.Inheritance(keyToken, value));
            }
            else
            {

                RSONValue value = parseAndEvaluateExpression();
                if (value.type == RSONValue::Type::Invalid)
                    return RResult::Fail;

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
                if (tokenToInt(token, value) != RResult::Ok)
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

    RSONValue ParserImpl::parseIdentifier()
    {
        Token identifier = advance();
        const UnownedStringSlice stringSlice = identifier.stringSlice;

        if (stringSlice == "null")
            return RSONValue::MakeNull();
        else if (stringSlice == "true")
            return RSONValue::MakeBool(true);
        else if (stringSlice == "false")
            return RSONValue::MakeBool(false);

        if (!RR::Common::IsSet(flags_, Flags::ParentExpressionParse))
        {
            getSink().Diagnose(identifier, Diagnostics::undeclaredIdentifier, stringSlice);
            return RSONValue {};
        }
        else
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