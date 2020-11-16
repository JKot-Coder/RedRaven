#include "ResourceImpl.hpp"

#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/TypeConversions.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            namespace
            {
                void GetOptimizedClearValue(Resource::BindFlags bindFlags, DXGI_FORMAT format, D3D12_CLEAR_VALUE*& value)
                {
                    if (!IsAny(bindFlags, Resource::BindFlags::RenderTarget | Resource::BindFlags::DepthStencil))
                    {
                        value = nullptr;
                        return;
                    }

                    if (IsSet(bindFlags, Resource::BindFlags::RenderTarget))
                    {
                        value->Format = format;

                        value->Color[0] = 0.0f;
                        value->Color[1] = 0.0f;
                        value->Color[2] = 0.0f;
                        value->Color[3] = 0.0f;
                    }

                    if (IsSet(bindFlags, Resource::BindFlags::DepthStencil))
                    {
                        value->Format = format;

                        value->DepthStencil.Depth = 1.0f;
                    }
                }

                D3D12_RESOURCE_DESC GetResourceDesc(const TextureDescription& resourceDesc, Resource::BindFlags bindFlags)
                {
                    DXGI_FORMAT format = TypeConversions::ResourceFormat(resourceDesc.format);

                    D3D12_RESOURCE_DESC desc;
                    switch (resourceDesc.dimension)
                    {
                    case TextureDimension::Texture1D:
                        desc = CD3DX12_RESOURCE_DESC::Tex1D(format, resourceDesc.width, resourceDesc.arraySize, resourceDesc.mipLevels);
                        break;
                    case TextureDimension::Texture2D:
                    case TextureDimension::Texture2DMS:
                        desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize, resourceDesc.mipLevels, resourceDesc.sampleCount);
                        break;
                    case TextureDimension::Texture3D:
                        desc = CD3DX12_RESOURCE_DESC::Tex3D(format, resourceDesc.width, resourceDesc.height, resourceDesc.depth, resourceDesc.mipLevels);
                        break;
                    case TextureDimension::TextureCube:
                        desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize * 6, resourceDesc.mipLevels);
                        break;
                    default:
                        LOG_FATAL("Unsupported texture dimension");
                    }

                    desc.Flags = TypeConversions::ResourceFlags(bindFlags);

                    if (resourceDesc.format.IsDepth() && IsAny(bindFlags, Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess))
                        format = TypeConversions::GetTypelessFormatFromDepthFormat(resourceDesc.format);

                    desc.Format = format;
                    return desc;
                }
            }

            Result ResourceImpl::Init(const ComSharedPtr<ID3D12Device>& device, const TextureDescription& resourceDesc, Resource::BindFlags bindFlags, const U8String& name)
            {
                ASSERT(device)
                // TextureDesc ASSERT checks done on Texture initialization;

                const DXGI_FORMAT format = TypeConversions::ResourceFormat(resourceDesc.format);
                ASSERT(format != DXGI_FORMAT_UNKNOWN)

                D3D12_CLEAR_VALUE optimizedClearValue;
                D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
                GetOptimizedClearValue(bindFlags, format, pOptimizedClearValue);

                const D3D12_RESOURCE_DESC& desc = GetResourceDesc(resourceDesc, bindFlags);

                D3DCallMsg(
                    device->CreateCommittedResource(
                        &DefaultHeapProps,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_COMMON,
                        pOptimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())),
                    "ResourceImpl::CreateCommittedResource");

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                return Result::Ok;
            }
        }
    }
}