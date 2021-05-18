#pragma once

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

namespace D3D12MA
{
    class Allocation;
}

namespace OpenDemo
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

                void ReleaseD3DObjects();

                void Init(const Buffer& resource);
                void Init(const Texture& resource);
                void Init(const GpuResourceDescription& resourceDesc, GpuResourceCpuAccess cpuAccess, const U8String& name);

                void Init(const ComSharedPtr<ID3D12Resource>& resource, D3D12MA::Allocation* allocation, const U8String& name);

                const ComSharedPtr<ID3D12Resource>& GetD3DObject() const { return D3DResource_; }

                void Map(uint32_t subresource, const D3D12_RANGE& readRange, void*& memory);
                void Unmap(uint32_t subresource, const D3D12_RANGE& writtenRange);

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
                D3D12MA::Allocation* allocation_;
            };
        }
    }
}