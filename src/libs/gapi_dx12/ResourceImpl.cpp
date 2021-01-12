#include "ResourceImpl.hpp"

#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/TypeConversions.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                void GetOptimizedClearValue(GpuResourceBindFlags bindFlags, DXGI_FORMAT format, D3D12_CLEAR_VALUE*& value)
                {
                    if (!IsAny(bindFlags, GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::DepthStencil))
                    {
                        value = nullptr;
                        return;
                    }

                    if (IsSet(bindFlags, GpuResourceBindFlags::RenderTarget))
                    {
                        value->Format = format;

                        value->Color[0] = 0.0f;
                        value->Color[1] = 0.0f;
                        value->Color[2] = 0.0f;
                        value->Color[3] = 0.0f;
                    }

                    if (IsSet(bindFlags, GpuResourceBindFlags::DepthStencil))
                    {
                        value->Format = format;

                        value->DepthStencil.Depth = 1.0f;
                    }
                }

                D3D12_RESOURCE_DESC GetResourceDesc(const TextureDescription& resourceDesc, GpuResourceBindFlags bindFlags)
                {
                    DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);

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

                    desc.Flags = TypeConversions::GetResourceFlags(bindFlags);

                    if (GpuResourceFormatInfo::IsDepth(resourceDesc.format) && IsAny(bindFlags, GpuResourceBindFlags::ShaderResource | GpuResourceBindFlags::UnorderedAccess))
                        format = TypeConversions::GetTypelessFormatFromDepthFormat(resourceDesc.format);

                    desc.Format = format;
                    return desc;
                }
            }

            void ResourceImpl::ReleaseD3DObjects(ResourceReleaseContext& releaseContext)
            {
                releaseContext.DeferredD3DResourceRelease(D3DResource_);
            }

            Result ResourceImpl::Init(const ComSharedPtr<ID3D12Device>& device, const TextureDescription& resourceDesc, GpuResourceBindFlags bindFlags, const U8String& name)
            {
                ASSERT(device)
                // TextureDesc ASSERT checks done on Texture initialization;

                const DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);
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

            Result ResourceImpl::Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& resourceDesc, GpuResourceBindFlags bindFlags, const U8String& name)
            {
                ASSERT(resource);
                // TextureDesc ASSERT checks done on Texture initialization;

                const DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);
                ASSERT(format != DXGI_FORMAT_UNKNOWN);

                const D3D12_RESOURCE_DESC& desc = resource->GetDesc();
                ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
                ASSERT(desc.Format == format);

                D3DResource_ = resource;
                D3DUtils::SetAPIName(D3DResource_.get(), name);

                return Result::Ok;
            }
        }
    }
}