#pragma once

#include "gapi/PipelineState.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    class IRenderDevice;
    class IPipelineState;
}
namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    class PipelineStateImpl final : public GAPI::IPipelineState
    {
    public:
        PipelineStateImpl() = default;
        ~PipelineStateImpl();

        void Init(DL::IRenderDevice* device, GAPI::PipelineState& resource);
    private:
        DL::RefCntAutoPtr<DL::IPipelineState> pso;
    };
}