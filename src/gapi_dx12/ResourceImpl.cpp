#include "ResourceImpl.hpp"

#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/TypeConversions.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            Result ResourceImpl::Init(const ComSharedPtr<ID3D12Device> device, const Texture::TextureDesc& resourceDesc, const Texture::BindFlags bindFlags, const U8String& name)
            {
                ASSERT(device)

                ASSERT(resourceDesc.width > 0)
                ASSERT(resourceDesc.height > 0)
                ASSERT(resourceDesc.depth > 0)

                ASSERT(resourceDesc.mipLevels > 0)
                ASSERT(resourceDesc.sampleCount > 0)
                ASSERT(resourceDesc.arraySize > 0)

                ASSERT(
                    (resourceDesc.sampleCount > 1 && resourceDesc.type == Texture::Type::Texture2DMS)
                    || (resourceDesc.sampleCount == 1 && resourceDesc.type != Texture::Type::Texture2DMS));

                switch (resourceDesc.type)
                {
                case Texture::Type::Texture1D:
                    ASSERT(resourceDesc.height == 1)
                    ASSERT(resourceDesc.depth == 1)
                    break;
                case Texture::Type::Texture2D:
                case Texture::Type::Texture2DMS:
                case Texture::Type::TextureCube:
                    ASSERT(resourceDesc.depth == 1)
                    break;
                case Texture::Type::Texture3D:
                    ASSERT(resourceDesc.arraySize == 1)
                    break;
                default:
                    LOG_FATAL("Unsupported texture type");
                }

                D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
                D3D12_RESOURCE_STATES initialResourceState;
                D3D12_CLEAR_VALUE optimizedClearValue;
                DXGI_FORMAT format = TypeConversions::ResourceFormat(resourceDesc.format);

                ASSERT(format != DXGI_FORMAT_UNKNOWN)

                D3D12_RESOURCE_DESC desc;
                switch (resourceDesc.type)
                {
                case Texture::Type::Texture1D:
                    desc = CD3DX12_RESOURCE_DESC::Tex1D(format, resourceDesc.width, resourceDesc.arraySize, resourceDesc.mipLevels);
                    break;
                case Texture::Type::Texture2D:
                case Texture::Type::Texture2DMS:
                    desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize, resourceDesc.mipLevels, resourceDesc.sampleCount);
                    break;
                case Texture::Type::Texture3D:
                    desc = CD3DX12_RESOURCE_DESC::Tex3D(format, resourceDesc.width, resourceDesc.height, resourceDesc.depth, resourceDesc.mipLevels);
                    break;
                case Texture::Type::TextureCube:
                    desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize * 6, resourceDesc.mipLevels);
                    break;
                default:
                    LOG_FATAL("Unsupported texture type");
                }

                desc.Flags = TypeConversions::ResourceFlags(bindFlags);

                D3DCallMsg(
                    device->CreateCommittedResource(
                        &DefaultHeapProps,
                        heapFlags,
                        &desc,
                        initialResourceState,
                        &optimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())),
                    "ResourceImpl::CreateCommittedResource");

                return Result::Ok;
            }
        }
    }
}