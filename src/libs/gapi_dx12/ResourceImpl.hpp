#pragma once

#include "gapi/Texture.hpp"
#include "gapi/Buffer.hpp"

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

                Result Init(const Texture& resource, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint);
                Result Init(const TextureDescription& resourceDesc, const GpuResourceBindFlags bindFlags, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint, const U8String& name);

                Result Init(const BufferDescription& resourceDesc, const GpuResourceBindFlags bindFlags, const U8String& name);

                // Only used for initialize swapchain texture
                Result Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& desc, const U8String& name);

                const ComSharedPtr<ID3D12Resource>& GetD3DObject() const { return D3DResource_; }

            private:
                Result performInitialUpload(const std::vector<TextureSubresourceFootprint>& subresourcesFootprint);

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
            };
        }
    }
}