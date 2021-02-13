#pragma once

#include "gapi/Buffer.hpp"
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

                void Init(const Texture& resource);
                void Init(const TextureDescription& resourceDesc, const GpuResourceBindFlags bindFlags, GpuResourceCpuAccess cpuAccess, const U8String& name);

                void Init(const Buffer& resource);
                void Init(const BufferDescription& resourceDesc, const GpuResourceBindFlags bindFlags, GpuResourceCpuAccess cpuAccess, const U8String& name);

                // Only used for initialize swapchain texture
                void Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& desc, const U8String& name);

                const ComSharedPtr<ID3D12Resource>& GetD3DObject() const { return D3DResource_; }

                void Map(uint32_t subresource, const D3D12_RANGE& range, void*& memory);

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
            };
        }
    }
}