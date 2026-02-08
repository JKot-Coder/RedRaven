#pragma once

#include "render/ResourcePointers.hpp"

namespace RR::Render
{
    class RenderTarget
    {
    public:
        RenderTarget(const GAPI::GpuResourceDesc& desc);

        const ShaderResourceView* GetSRV(uint32_t mipLevel = 0, uint32_t mipCount = MaxPossible, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const RenderTargetView* GetRTV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const DepthStencilView* GetDSV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);
        const UnorderedAccessView* GetUAV(uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t numArraySlices = MaxPossible, GpuResourceFormat format = GpuResourceFormat::Unknown);

    private:
        TextureUniquePtr texture_;

        // TODO Use one map for all views
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, ShaderResourceViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> srvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, RenderTargetViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> rtvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, DepthStencilViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> dsvs_;
        absl::flat_hash_map<GAPI::GpuResourceViewDesc, UnorderedAccessViewUniquePtr, GAPI::GpuResourceViewDesc::HashFunc> uavs_;
    };
}