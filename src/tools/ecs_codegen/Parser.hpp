#pragma once

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace ParseTools
    {
        struct CompileContext;
        struct TokenSpan;
    }

    class ParserImpl;

    class Parser final : Common::NonCopyable
    {
    public:
        ~Parser();
        Parser() = delete;
        Parser(const ParseTools::TokenSpan& tokenSpan, const std::shared_ptr<ParseTools::CompileContext>& context);

        Common::RResult Parse();

    private:
        std::unique_ptr<ParserImpl> impl_;
    };
}