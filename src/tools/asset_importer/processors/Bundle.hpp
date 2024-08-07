#pragma once

#include "Processor.hpp"

namespace RR::AssetImporter::Processors
{
    class Bundle : public Processor
    {
    public:
        std::vector<std::string> GetListOfExtensions() const override;
        RR::Common::RResult Process(const Asset& assconstet, const ProcessorContext& context, std::vector<ProcessorOutput>& outputs) const override;
    };
}