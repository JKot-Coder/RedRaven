#pragma once

#include "gapi/Texture.hpp"

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
                ~ResourceImpl() = default;

                void ReleaseD3DObjects();

                Result Init(const ComSharedPtr<ID3D12Device>& device, const TextureDescription& desc, const GpuResourceBindFlags bindFlags, const U8String& name);
                Result Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& desc, const GpuResourceBindFlags bindFlags, const U8String& name);

                const ComSharedPtr<ID3D12Resource>& GetD3DObject() const { return D3DResource_; }

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
            };
        }
    }
}