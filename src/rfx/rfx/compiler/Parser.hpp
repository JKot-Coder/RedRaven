#pragma once

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace Rfx
    {
        class ParserImpl;
        struct CompileContext;
        struct TokenSpan;
        struct RSONValue;

        class Parser final : Common::NonCopyable
        {
        public:
            ~Parser();
            Parser() = delete;
            Parser(const TokenSpan& tokenSpan, const std::shared_ptr<CompileContext>& context);

            Common::RResult Parse(RSONValue& root);

        private:
            std::unique_ptr<ParserImpl> impl_;
        };
    }
}