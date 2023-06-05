#include "EffectParser.hpp"

#include "common/ComPtr.hpp"
#include "common/LinearAllocator.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/ASTBuilder.hpp"
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

        auto astBuilder = std::make_shared<ASTBuilder>();
        auto context = std::make_shared<CompileContext>();

        Parser parser(SourceView::Create(source), context);
        RR_RETURN_ON_FAIL(parser.Parse(astBuilder));

        auto stream = source->GetStream();

        std::cout << "test"
                  << "\n";

        return result;
    }
}