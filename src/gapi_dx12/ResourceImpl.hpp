#pragma once

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class ResourceImpl final
            {
            public:
                ResourceImpl() = default;
                Result Init(const ComSharedPtr<ID3D12Device>& device, const TextureDescription& desc, const Resource::BindFlags bindFlags, const U8String& name);

                const ComSharedPtr<ID3D12Resource>& getD3DObject() const { return D3DResource_; } 

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
            };
        }
    }
}