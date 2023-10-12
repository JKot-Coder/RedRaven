#include "EffectCompiler.hpp"

#include "compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/RSONValue.hpp"

namespace RR::Rfx
{
    EffectCompiler::EffectCompiler(const std::shared_ptr<CompileContext>& context) : context_(context) { }
    EffectCompiler::~EffectCompiler() { }

    DiagnosticSink& EffectCompiler::GetDiagnosticSink() const
    {
        return context_->sink;
    }

    void EffectCompiler::Compile(const RSONValue& rson)
    {
        std::ignore = rson;
        //rson["technique"]
    }

}