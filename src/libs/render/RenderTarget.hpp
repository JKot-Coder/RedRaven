#pragma once

#include "render/ResourcePointers.hpp"

#include "absl/container/flat_hash_map.h"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"

namespace RR::Render
{
    class RenderTarget
    {
    public:
        static constexpr uint32_t MaxPossible = 0xFFFFFF;

    public:
        RenderTarget(const GAPI::GpuResourceDesc& desc);

        const GAPI::ShaderResourceView* GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GAPI::GpuResourceFormat format = GAPI::GpuResourceFormat::Unknown);
        const GAPI::RenderTargetView* GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GAPI::GpuResourceFormat format = GAPI::GpuResourceFormat::Unknown);
        const GAPI::DepthStencilView* GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GAPI::GpuResourceFormat format = GAPI::GpuResourceFormat::Unknown);
        const GAPI::UnorderedAccessView* GetUAV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GAPI::GpuResourceFormat format = GAPI::GpuResourceFormat::Unknown);

    private:
        TextureUniquePtr texture_;

        // TODO Use one map for all views
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, ShaderResourceViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> srvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, RenderTargetViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> rtvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, DepthStencilViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> dsvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, UnorderedAccessViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> uavs_;
    };
}