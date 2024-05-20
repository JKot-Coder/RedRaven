#pragma once

#include "Processor.hpp"
#include "common/ComPtr.hpp"

namespace RR::Rfx
{
    class ICompiler;
}

namespace RR::AssetImporter::Processors
{
    class Effect : public Processor
    {
    public:
        Effect();

        std::vector<U8String> GetListOfExtensions() const override;
        RR::Common::RResult Process(const Asset& asset, const ProcessorContext& context, std::vector<ProcessorOutput>& outputs) const override;

    private:
        RR::Common::ComPtr<RR::Rfx::ICompiler> compiler_;
    };
}