#include "EffectParser.hpp"

#include "common/ComPtr.hpp"
#include "common/LinearAllocator.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/core/SourceLocation.hpp"

#include "Parser.hpp"

#include <iostream>

namespace RR::Rfx
{
    using RR::Common::ComPtr;

    RfxResult EffectParser::Parse(std::shared_ptr<RR::Rfx::SourceFile>& source, ComPtr<IBlob>& output, ComPtr<IBlob>& diagnostic)
    {
        std::ignore = diagnostic;
        std::ignore = output;
        RfxResult result = RfxResult::Ok;

        auto context = std::make_shared<CompileContext>(false);

        Parser parser(SourceView::CreateFromSourceFile(source), context);
        RR_RETURN_ON_FAIL(parser.Parse());

        return result;
    }
}