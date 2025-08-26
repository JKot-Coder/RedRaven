#pragma once

#include "gapi/Limits.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/ForwardDeclarations.hpp"
#include "gapi/PipelineState.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Render
{
    using PsoHashType = uint64_t;
    constexpr size_t PsoHashBits = 64;

    struct EvaluateGraphicsParams
	{
        GAPI::PrimitiveTopology primitiveTopology;
        uint32_t renderTargetCount = 0;
        eastl::array<GAPI::GpuResourceFormat, GAPI::MAX_RENDER_TARGETS_COUNT> renderTargetFormats;
        GAPI::GpuResourceFormat depthStencilFormat;
    };

    class Effect
    {
    public:

        using UniquePtr = eastl::unique_ptr<Effect>;

        ~Effect();

    private:
        friend class DeviceContext;

        Effect(const std::string& name);

        static UniquePtr Create(const std::string& name)
        {
            return UniquePtr(new Effect(name));
        }

        GAPI::PipelineState* EvaluateGraphicsPipelineState(const EvaluateGraphicsParams& params);

    private:
        absl::flat_hash_map<PsoHashType, eastl::unique_ptr<GAPI::PipelineState>> pipelineStates;
    };



}