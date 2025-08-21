#pragma once

#include "gapi/PipelineState.hpp"

namespace Diligent
{
    class IRenderDevice;
}
namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    class PipelineStateImpl final : public GAPI::IPipelineState
    {
    public:
        PipelineStateImpl() = default;
        ~PipelineStateImpl() override;

        void Init(DL::IRenderDevice* device, GAPI::PipelineState& resource) const;
    };
}