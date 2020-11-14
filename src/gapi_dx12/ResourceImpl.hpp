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
                Result Init(const ComSharedPtr<ID3D12Device> device, const Texture::Description& desc, const Resource::BindFlags bindFlags, const U8String& name);

            private:
                ComSharedPtr<ID3D12Resource> D3DResource_;
            };
        }
    }
}