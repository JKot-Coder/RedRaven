#include "EffectParser.hpp"

#include "common/ComPtr.hpp"
#include "common/LinearAllocator.hpp"
#include "common/Result.hpp"
#include "compiler/DiagnosticSink.hpp"
#include "compiler/ASTBuilder.hpp"
#include "core/SourceLocation.hpp"

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

        auto allocator = std::make_shared<Common::LinearAllocator>(2048);
        auto diagnosticSink = std::make_shared<DiagnosticSink>();
        auto astBuilder = std::make_shared<ASTBuilder>();

        Parser parser(SourceView::Create(source), allocator, diagnosticSink);
        RR_RETURN_ON_FAIL(parser.Parse(astBuilder));

        auto stream = source->GetStream();

        std::cout << "test"
                  << "\n";

        return result;
    }
}