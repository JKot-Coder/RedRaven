#pragma once

namespace RR
{
    namespace Common
    {
        enum class [[nodiscard]] RResult : int32_t;
    }

    namespace GAPI
    {
        struct RasterizerDesc;
        struct RTBlendStateDesc;
    }

    namespace Rfx
    {
        using RResult = Common::RResult;
        struct UnownedStringSlice;

        class DiagnosticSink;
        struct CompileContext;
        struct RSONValue;
        class EffectCompiler
        {
        public:
            EffectCompiler(const std::shared_ptr<CompileContext>& context);
            ~EffectCompiler();

            void Compile(RSONValue rson);

        private:
            DiagnosticSink& getSink() const;

            RResult parseTehnique(RSONValue tehnique) const;
            RResult parsePass(RSONValue pass) const;
            RResult parseRasterizerDesc(UnownedStringSlice key, RSONValue value, GAPI::RasterizerDesc& rasterizerDesk) const;
            RResult parseBlendStateDesc(UnownedStringSlice key, RSONValue value, GAPI::RTBlendStateDesc& blendDesk) const;
            RResult parseRenderState(RSONValue renderState, GAPI::RasterizerDesc& rasterizerDesc) const;

            RResult throwInvalidKey(UnownedStringSlice key, RSONValue value) const;
            RResult throwInvalidKey(UnownedStringSlice key, UnownedStringSlice value) const;

        private:
            std::shared_ptr<CompileContext> context_;
        };

    }
}