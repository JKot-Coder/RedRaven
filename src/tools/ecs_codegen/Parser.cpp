#include "Parser.hpp"

#include "parse_tools/DiagnosticCore.hpp"
#include "parse_tools/Lexer.hpp"
#include "parse_tools/Token.hpp"
#include "parse_tools/core/CompileContext.hpp"

#include "common/Result.hpp"

#include <iostream>

#include "parse_tools/core/StringEscapeUtil.hpp"
namespace RR
{
    using namespace Common;
    using namespace ParseTools;

    namespace
    {
        void qweqwe(const std::shared_ptr<const SourceView>& sourceView, std::shared_ptr<const SourceView>& outSourceView, HumaneSourceLocation& outHumaneSourceLoc)
        {
            outSourceView = sourceView;

            while (outSourceView->GetInitiatingSourceLocation().GetSourceView() &&
                   outSourceView->GetPathInfo().type != PathInfo::Type::Normal &&
                   outSourceView->GetPathInfo().type != PathInfo::Type::Split)
            {
                outHumaneSourceLoc = outSourceView->GetInitiatingSourceLocation().humaneSourceLoc;
                outSourceView = outSourceView->GetInitiatingSourceLocation().GetSourceView();
            }
        }
    }

    template <int IndentCount>
    class SourceWriter
    {
    public:
        SourceWriter() { }
        virtual ~SourceWriter() { }
        std::string& GetOutputString() { return buffer_; };

    protected:
        inline void indent() { indentLevel_++; }
        inline void dedent()
        {
            ASSERT(indentLevel_ > 0);

            if (indentLevel_ == 0)
                return;

            indentLevel_--;
        }

        void writeIndent()
        {
            for (uint32_t i = 0; i < indentLevel_ * IndentCount; i++)
                push_back(' ');
        }

        void push_back(char ch) { buffer_.push_back(ch); }
        void append(const std::string& str) { buffer_.append(str); }
        void append(const char* str, size_t count) { buffer_.append(str, count); }
        template <class InputIterator>
        void append(InputIterator first, InputIterator last) { buffer_.append(first, last); }

    protected:
        uint32_t indentLevel_ = 0;
        std::string buffer_;
    };

    class TokenWriter final : public SourceWriter<4>
    {
    public:
        TokenWriter(bool onlyRelativePaths) : onlyRelativePaths_(onlyRelativePaths) { }

        void Emit(const Token& token)
        {
            if (token.type == Token::Type::RBrace)
                dedent();

            if (Common::IsSet(token.flags, Token::Flags::AtStartOfLine))
            {
                if (!GetOutputString().empty()) // Skip new line at the wery begining
                    push_back('\n');

                if (currentSourceFile_ != token.sourceLocation.GetSourceView()->GetSourceFile())
                {
                    HumaneSourceLocation humaleLoc = token.sourceLocation.humaneSourceLoc;
                    std::shared_ptr<const SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);
                    ASSERT(sourceView);

                    line_ = humaleLoc.line;

                    const auto sourceViewUniqueIdentity = sourceView->GetSourceFile()->GetPathInfo().getMostUniqueIdentity();
                    if (!currentSourceFile_ || currentUniqueIndentity_ != sourceViewUniqueIdentity)
                    {
                        currentUniqueIndentity_ = sourceViewUniqueIdentity;
                        auto const path = onlyRelativePaths_ ? sourceView->GetSourceFile()->GetPathInfo().foundPath : sourceViewUniqueIdentity;

                        std::string escapedPath;
                        StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, path, escapedPath);

                        currentSourceFile_ = token.sourceLocation.GetSourceView()->GetSourceFile();
                        writeIndent();
                        append(fmt::format("#line {} \"{}\"\n", line_, escapedPath));
                    }
                    else
                    {
                        writeIndent();
                        append(fmt::format("#line {}\n", line_));
                    }
                }
                else
                {
                    HumaneSourceLocation humaleLoc = token.sourceLocation.humaneSourceLoc;
                    std::shared_ptr<const SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);
                    ASSERT(sourceView);

                    //  line_ = token.humaneSourceLocation.line;

                    constexpr int maxNewLinesInARow = 3;
                    if (humaleLoc.line - line_ <= maxNewLinesInARow)
                    {
                        while (line_ < humaleLoc.line)
                        {
                            line_++;
                            push_back('\n');
                        }
                    }

                    if (humaleLoc.line != line_)
                    {
                        line_ = humaleLoc.line;
                        writeIndent();
                        append(fmt::format("#line {}\n", line_));
                    }
                }

                line_++;
                writeIndent();
            }
            else if (Common::IsSet(token.flags, Token::Flags::AfterWhitespace))
            {
                push_back(' ');
            }

            switch (token.type)
            {
                case Token::Type::StringLiteral:
                case Token::Type::CharLiteral:
                {
                    std::string escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, token.stringSlice, escapedToken);

                    auto appendQuoted = [](char quotingChar, const std::string& token) { return quotingChar + token + quotingChar; };

                    char quotingChar = token.type == Token::Type::StringLiteral ? '\"' : '\'';
                    append(appendQuoted(quotingChar, escapedToken));
                    break;
                }
                default:
                    append(token.stringSlice.begin(), token.stringSlice.end());
                    break;
            }

            if (token.type == Token::Type::LBrace)
                indent();
        }

    private:
        uint32_t line_ = 1;
        std::string currentUniqueIndentity_;
        std::shared_ptr<SourceView> currentSourceView_;
        std::shared_ptr<SourceFile> currentSourceFile_;
        bool onlyRelativePaths_;
    };

    class ParserImpl final : Common::NonCopyable
    {
    public:
        ParserImpl() = delete;
        ParserImpl(const TokenSpan& tokenSpan,
                   const std::shared_ptr<CompileContext>& context)
            : context_(context),
              tokenSpan_(tokenSpan),
              currentToken_(tokenSpan_.begin())
        {
            ASSERT(context);
        }

        RResult Parse();

    private:
        DiagnosticSink& getSink() const { return context_->sink; }
        auto& getAllocator() const { return context_->allocator; }

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

        bool advanceIf(std::initializer_list<Token::Type> expected, Token& outToken);
        bool inline advanceIf(Token::Type expected, Token& outToken) { return advanceIf({expected}, outToken); }
        bool inline advanceIf(Token::Type expected)
        {
            Token token;
            return advanceIf(expected, token);
        }

        template <typename T>
        UnownedStringSlice allocateString(T begin, T end)
        {
            size_t size = std::distance(begin, end);
            const auto buf = (char*)getAllocator().Allocate(size);
            std::copy(begin, end, buf);
            return UnownedStringSlice(buf, buf + size);
        }

        RResult expect(std::initializer_list<Token::Type> expected, Token& outToken);
        RResult expect(Token::Type expected, Token& outToken) { return expect({expected}, outToken); };
        RResult expect(Token::Type expected)
        {
            Token token;
            return expect(expected, token);
        };

        RResult skipScope();
        RResult skipTemplate();
        RResult skipScope(Token::Type beginTokenType, Token::Type endTokenType);

        RResult tryParseFunction();
        RResult tryParseTemplate();
        RResult parseDeclaration();
        RResult parseValueType(std::string& outType);
        void skipUntil(Token::Type type) { skipUntil({type}); }
        void skipUntil(std::initializer_list<Token::Type> expected);

    private:
        std::shared_ptr<CompileContext> context_;
        TokenSpan tokenSpan_;
        TokenSpan::const_iterator currentToken_;
    };

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

    RResult ParserImpl::expect(std::initializer_list<Token::Type> expected, Token& out)
    {
        const auto lookAheadTokenType = peekTokenType();

        for (const auto& expectedType : expected)
            if (expectedType == lookAheadTokenType)
            {
                out = advance();
                return RResult::Ok;
            }

        const std::string& tokensString = fmt::format("{}", fmt::join(expected, " or "));
        getSink().Diagnose(peekToken(), Diagnostics::unexpectedTokenExpectedTokenType, peekTokenType(), tokensString);
        return RResult::Fail;
    }

    RResult ParserImpl::skipScope()
    {
        RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));

        int32_t scopeDepth = 1;

        while (true)
        {
            switch (peekTokenType())
            {
                case Token::Type::EndOfFile:
                    getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                    return RResult::Unexpected;
                case Token::Type::LBrace:
                    scopeDepth++;
                    advance();
                    continue;
                case Token::Type::RBrace:
                    scopeDepth--;
                    advance();

                    if (scopeDepth == 0)
                        break;

                    continue;
                default:
                    advance();
                    continue;
            }

            break;
        }

        return RResult::Ok;
    }

    RResult ParserImpl::skipTemplate()
    {
        RR_RETURN_ON_FAIL(expect(Token::Type::OpLess));
        int depth = 1;
        while (true)
        {
            switch(peekTokenType())
            {
                case Token::Type::OpLess:
                {
                    advance();
                    depth++;
                    continue;
                }
                case Token::Type::OpGreater:
                {
                    depth--;
                    if (depth <= 0)
                        break;
                    advance();
                    continue;
                }
                case Token::Type::OpRsh:
                {
                    depth -= 2;
                    if (depth <= 0)
                        break;

                    advance();
                    continue;
                }
                case Token::Type::EndOfFile:
                    break;
                default:
                    advance();
                    continue;
            }

            break;
        }

        return RResult::Ok;
    };

    RResult ParserImpl::skipScope(Token::Type beginTokenType, Token::Type endTokenType)
    {
        //         template <bool isFloatPoint = std::is_floating_point<T>::value, typename = std::enable_if_t<isFloatPoint>>
        RR_RETURN_ON_FAIL(expect(beginTokenType));
        int depth = 1;
        while (true)
        {
            if (peekTokenType() == beginTokenType)
            {
                advance();
                depth++;
                continue;
            }

            if (peekTokenType() == endTokenType)
            {
                depth--;
                if (depth == 0)
                    break;
                advance();
                continue;
            }

            if(peekTokenType() == Token::Type::EndOfFile)
                break;

            advance();
            continue;
        }

        return RResult::Ok;
    };

    RResult ParserImpl::tryParseFunction()
    {
        auto beginTokenIt = currentToken_;

        while(true)
        {
            switch (peekTokenType())
            {
                default:
                    advance();
                    continue;

                case Token::Type::Identifier:
                {
                    if(peekToken().stringSlice == "struct" ||
                       peekToken().stringSlice == "class" ||
                       peekToken().stringSlice == "union" ||
                       peekToken().stringSlice == "enum")
                    {
                        currentToken_ = beginTokenIt;
                        return RResult::False;
                    }

                    advance();
                    continue;
                }

                case Token::Type::LParent:
                {
                    skipUntil(Token::Type::RParent);
                    RR_RETURN_ON_FAIL(expect(Token::Type::RParent));
                    break;
                }

                case Token::Type::OpAssign:
                case Token::Type::Semicolon:
                case Token::Type::EndOfFile:
                    currentToken_ = beginTokenIt;
                    return RResult::False;
            }

            break;
        }

        // It's a function and we stop at ')' and wee need skip qualifiers until ; or {
        while(true)
        {
            switch (peekTokenType())
            {
                default:
                    advance();
                    continue;
                case Token::Type::Semicolon:
                case Token::Type::EndOfFile:
                case Token::Type::LBrace:
                    break;
            }
            break;
        }

        if (peekTokenType() == Token::Type::LBrace)
        {
            RR_RETURN_ON_FAIL(skipScope());
        }
        else
            RR_RETURN_ON_FAIL(expect(Token::Type::Semicolon));

        return RResult::Ok;
    }

    RResult ParserImpl::tryParseTemplate()
    {
        if (peekTokenType() != Token::Type::Identifier || peekToken().stringSlice != "template")
            return RResult::False;

        Token ignore;
        advance(); // skip keyword
        RR_RETURN_ON_FAIL(skipTemplate());
        RR_RETURN_ON_FAIL(expect({Token::Type::OpGreater, Token::Type::OpRsh}, ignore));

        return RResult::Ok;
    }

    RResult ParserImpl::parseValueType(std::string& outType)
    {
        while (true)
        {
            Token token;
            RR_RETURN_ON_FAIL(expect(Token::Type::Identifier, token));

            outType.append(token.stringSlice.begin(), token.stringSlice.end());

            if (peekTokenType() == Token::Type::Scope)
            {
                outType.push_back(':');
                outType.push_back(':');
                advance();
                continue;
            }

            if (peekTokenType() == Token::Type::OpLess)
            {
                outType.push_back('<');

                Token ignore;
                RR_RETURN_ON_FAIL(skipTemplate());
                RR_RETURN_ON_FAIL(expect({Token::Type::OpGreater, Token::Type::OpRsh}, ignore));
                outType.push_back('>');
            }

            break;
        }

        return RResult::Ok;
    }

    RResult ParserImpl::parseDeclaration()
    {
        {
            // Check if it's a function
            const RResult result = tryParseFunction();

            RR_RETURN_ON_FAIL(result); // Something went wrong
            if (result != RResult::False) // It's a function
                return result;

            ASSERT(result == RResult::False);
        }
        {
            // Check if it's a template
            const RResult result = tryParseTemplate();

            RR_RETURN_ON_FAIL(result); // Something went wrong
            if (result != RResult::False) // It's a template
                return result;

            ASSERT(result == RResult::False);
        }

        auto trySkipAlignas = [this]() -> RResult {
            if (peekTokenType() == Token::Type::Identifier && peekToken().stringSlice == "alignas")
            {
                advance();
                RR_RETURN_ON_FAIL(skipScope(Token::Type::LParent, Token::Type::RParent));
                RR_RETURN_ON_FAIL(expect(Token::Type::RParent));
                return RResult::Ok;
            }
            return RResult::False;
        };

        while (true)
        {
            switch (peekTokenType())
            {
                case Token::Type::Identifier:
                {
                    if (peekToken().stringSlice == "const" ||
                        peekToken().stringSlice == "static" ||
                        peekToken().stringSlice == "volatile" ||
                        peekToken().stringSlice == "extern" ||
                        peekToken().stringSlice == "inline" ||
                        peekToken().stringSlice == "register" ||
                        peekToken().stringSlice == "restrict" ||
                        peekToken().stringSlice == "thread_local" ||
                        peekToken().stringSlice == "mutable" ||
                        peekToken().stringSlice == "constexpr")
                    {
                        advance();
                        continue;
                    }
                    {
                        RResult result = trySkipAlignas();
                        RR_RETURN_ON_FAIL(result);
                        if (result == RResult::Ok)
                            continue;
                    }

                    break;
                }
                default:
                    getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                    return RResult::Unexpected;
            }

            break;
        }

        auto parseIdentifier = [this](const std::string& type) -> RResult {
            Token name;

            while (true)
            {
                switch (peekTokenType())
                {
                    case Token::Type::Identifier:
                    {
                        if (peekToken().stringSlice == "const")
                        {
                            advance();
                            continue;
                        }

                        name = advance();
                        break;
                    }

                    case Token::Type::OpMul:
                    case Token::Type::OpBitAnd:
                    {
                        advance();
                        continue;
                    }

                    default:
                        getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                        return RResult::Unexpected;
                }

                break;
            }

            if (peekTokenType() == Token::Type::OpLess) // Template
            {
                Token ignore;
                RR_RETURN_ON_FAIL(skipTemplate());
                RR_RETURN_ON_FAIL(expect({Token::Type::OpGreater, Token::Type::OpRsh}, ignore));
            }

            if (peekTokenType() == Token::Type::LBracket) // Array
            {
                skipUntil(Token::Type::RBracket);
                RR_RETURN_ON_FAIL(expect(Token::Type::RBracket));
            }

            std::cout << "type: " << type << " name: " << name.stringSlice.asString() << std::endl;
            return RResult::Ok;
        };

        const Token typeToken = peekToken();
        std::string typeName;

        if (typeToken.stringSlice == "struct" ||
            typeToken.stringSlice == "class" ||
            typeToken.stringSlice == "union" ||
            typeToken.stringSlice == "enum")
        {
            enum class CType
            {
                Struct,
                Class,
                Union,
                Enum
            };

            CType cType = CType::Struct;
            if(typeToken.stringSlice == "class")
                cType = CType::Class;
            else if(typeToken.stringSlice == "union")
                cType = CType::Union;
            else if(typeToken.stringSlice == "enum")
                cType = CType::Enum;

            advance(); // Skip keyword

            RR_RETURN_ON_FAIL(trySkipAlignas());

            typeName = (peekTokenType() == Token::Type::Identifier) ? advance().stringSlice.asString() : "<UNNAMED>"; // C allows unnamed structs

            if(cType == CType::Class || cType == CType::Struct)
                std::cout << "new struct type: " << typeName << std::endl;

            if (peekTokenType() == Token::Type::OpLess) // Template // INCORRECT
            {
                skipUntil(Token::Type::OpGreater);
                RR_RETURN_ON_FAIL(expect(Token::Type::OpGreater));
            }

            // Inplace declaration of type
            // Maybe parse inners?
            if(peekTokenType() == Token::Type::Semicolon) // Forward declaration
                return RResult::Ok;

            if (cType == CType::Enum || cType == CType::Union)
            {
                RR_RETURN_ON_FAIL(skipScope());
            }
            else
            {
                RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));

                while (true)
                {
                    switch (peekTokenType())
                    {
                        case Token::Type::Identifier:
                        {
                            RR_RETURN_ON_FAIL(parseDeclaration());
                            continue;
                        }
                        case Token::Type::Semicolon:
                        {
                            advance();
                            continue;
                        }
                        default:
                            break;
                    }

                    break;
                }
                RR_RETURN_ON_FAIL(expect(Token::Type::RBrace));
            }

            if (peekTokenType() == Token::Type::Identifier) // Name is optional
                RR_RETURN_ON_FAIL(parseIdentifier(typeName));

        }else
        {
            RR_RETURN_ON_FAIL(parseValueType(typeName));
            RR_RETURN_ON_FAIL(parseIdentifier(typeName));
        }

        while (true)
        {
            if (peekTokenType() == Token::Type::Comma)
            {
                advance();
                RR_RETURN_ON_FAIL(parseIdentifier(typeName));
                continue;
            }

            break;
        }

        if (peekTokenType() == Token::Type::OpAssign)
            skipUntil(Token::Type::Semicolon);

        RR_RETURN_ON_FAIL(expect(Token::Type::Semicolon));
        return RResult::Ok;
    }

    RResult ParserImpl::Parse()
    {
        TokenWriter writer(false);

        int32_t scopeLevel = 0;
        while (true)
        {
            switch (peekTokenType())
            {
                case Token::Type::EndOfFile:
                    break;

                case Token::Type::LBrace:
                {
                    advance();
                    scopeLevel++;
                    continue;
                }

                case Token::Type::RBrace:
                {
                    scopeLevel--;

                    if (scopeLevel < 0)
                    {
                        getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                        return RResult::Unexpected;
                    }

                    advance();
                    continue;
                }
                case Token::Type::Identifier:
                {
                    if (peekToken().stringSlice == "namespace")
                    {
                        advance(); // Skip keyword

                        while (true) // Parse namespace name
                        {
                            RR_RETURN_ON_FAIL(expect(Token::Type::Identifier));
                            if(peekTokenType() == Token::Type::Scope)
                            {
                                advance();
                                continue;
                            }

                            break;
                        }

                        RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));
                        scopeLevel++;
                        continue;
                    }

                    if (peekToken().stringSlice == "typedef")
                    {
                        advance();
                        continue;
                    }


                    RR_RETURN_ON_FAIL(parseDeclaration());
                    continue;
                }
                default:
                    advance();
                    continue;
            }

            break;
        }

        RR_RETURN_ON_FAIL(expect(Token::Type::EndOfFile));

        std::cout << writer.GetOutputString() << std::endl;
        return RResult::Ok;
    }

    void ParserImpl::skipUntil(std::initializer_list<Token::Type> expected)
    {
        while (true)
        {
            if(peekTokenType() == Token::Type::EndOfFile)
                return;

            for (const auto expectedType : expected)
                if (peekTokenType() == expectedType)
                    return;

            advance();
        }
    }

    Parser::~Parser() { }

    Parser::Parser(const TokenSpan& tokenSpan,
                   const std::shared_ptr<CompileContext>& context)
        : impl_(std::make_unique<ParserImpl>(tokenSpan, context))
    {
    }

    RResult Parser::Parse()
    {
        ASSERT(impl_);
        return impl_->Parse();
    }
}
