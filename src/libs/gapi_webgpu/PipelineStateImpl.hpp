#pragma once

#include "gapi/PipelineState.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class PipelineStateImpl final : public GAPI::IPipelineState
    {
    public:
        PipelineStateImpl() = default;
        ~PipelineStateImpl();

        void Init(const wgpu::Device& device, GAPI::PipelineState& resource);

        wgpu::RenderPipeline GetRenderPipeline() const { return renderPipeline; }

    private:
        wgpu::RenderPipeline renderPipeline;
    };
}
