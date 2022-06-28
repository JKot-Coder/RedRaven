#pragma once

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

namespace D3D12MA
{
    class Allocation;
}

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceImpl final : public IGpuResource
            {
            public:
                ResourceImpl() = default;
                ~ResourceImpl();

                void Init(const Buffer& resource);
                void Init(const Texture& resource);
                void Init(const GpuResourceDescription& resourceDesc, GpuResourceUsage usage, const U8String& name);

                void Init(const ComSharedPtr<ID3D12Resource>& resource, D3D12MA::Allocation* allocation, const U8String& name);

                std::any GetRawHandle() const override { return D3DResource_.get(); }
                D3D12_RESOURCE_STATES GetDefaultResourceState() const { return defaultState_; }

                std::vector<CpuResourceData::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDescription& desc) const override;
                CpuResourceData::SubresourceFootprint GetSubresourceFootprintAt(const GpuResourceDescription& desc, uint32_t subresourceIndex) const override;

                void* Map() override;
                void Unmap() override;

                const ComSharedPtr<ID3D12Resource>& GetD3DObject() const { return D3DResource_; }

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
                D3D12MA::Allocation* allocation_;
                D3D12_RESOURCE_STATES defaultState_;
            };
        }
    }
}