#pragma once

namespace RR
{
    namespace Rfx
    {
        class DiagnosticSink;
        struct CompileContext;
        struct RSONValue;
        class EffectCompiler
        {
        public:
            EffectCompiler(const std::shared_ptr<CompileContext>& context);
            ~EffectCompiler();

            void Compile(const RSONValue& rson);

            DiagnosticSink& GetDiagnosticSink() const;

        private:
            std::shared_ptr<CompileContext> context_;
        };

    }
}