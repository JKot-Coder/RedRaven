#include "Parser.hpp"

#include "parse_tools/core/CompileContext.hpp"
#include "parse_tools/DiagnosticCore.hpp"
#include "parse_tools/Lexer.hpp"
#include "parse_tools/Token.hpp"

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
        ASSERT(peekTokenType() == Token::Type::LBrace);
        int32_t scopeDepth = 0;
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

                if (scopeDepth == 0)
                    break;

                advance();
                continue;
            default:
                advance();
                continue;
            }

            break;
        }

        return RResult::Ok;
    }

    RResult ParserImpl::Parse()
    {
        TokenWriter writer(false);

        int32_t scopeLevel = 0;
        while (true)
        {
            if (peekTokenType() == Token::Type::EndOfFile)
                break;

            if (peekTokenType() == Token::Type::Identifier)
            {
                if(peekToken().stringSlice == "namespace")
                {
                    RR_RETURN_ON_FAIL(expect(Token::Type::Identifier));
                    RR_RETURN_ON_FAIL(expect(Token::Type::LBrace));
                    scopeLevel++;
                    continue;
                } else if (peekToken().stringSlice == "typedef")
                {
                    advance();
                    continue;
                }
                else if (peekToken().stringSlice == "struct" ||
                         peekToken().stringSlice == "class" ||
                         peekToken().stringSlice == "enum" ||
                         peekToken().stringSlice == "union")
                {
                    // Skip body until semicolon
                    while (true)
                    {
                       switch (peekTokenType())
                       {
                        case Token::Type::LBrace:
                            RR_RETURN_ON_FAIL(skipScope());
                            continue;
                        case Token::Type::EndOfFile:
                            getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                            return RResult::Fail;
                        case Token::Type::Semicolon:
                            break;
                        default:
                            advance();
                            continue;
                       }

                       break;
                    }
                    RR_RETURN_ON_FAIL(expect(Token::Type::Semicolon));
                    continue;
                }
            } else if (peekTokenType() == Token::Type::RBrace)
            {
                scopeLevel--;

                if(scopeLevel < 0)
                {
                    getSink().Diagnose(peekToken(), Diagnostics::unexpectedToken, peekTokenType());
                    return RResult::Unexpected;
                }

                advance();
                continue;
            }

            writer.Emit(advance());
            continue;
        }

        RR_RETURN_ON_FAIL(expect(Token::Type::EndOfFile));
        
        std::cout<<writer.GetOutputString() << std::endl;
        return RResult::Ok;

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