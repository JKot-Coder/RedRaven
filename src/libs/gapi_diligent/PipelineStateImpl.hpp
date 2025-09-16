#pragma once

#include "gapi/PipelineState.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    struct IRenderDevice;
    struct IPipelineState;
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

        DL::IPipelineState* GetPipelineState() const { return pso.RawPtr(); }

    private:
        DL::RefCntAutoPtr<DL::IPipelineState> pso;
    };
}