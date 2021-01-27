#include "ResourceImpl.hpp"

#include "gapi_dx12/DeviceContext.hpp"
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

                    if (GpuResourceFormatInfo::IsDepth(resourceDesc.format) && IsAny(bindFlags, GpuResourceBindFlags::ShaderResource | GpuResourceBindFlags::UnorderedAccess))
                        format = TypeConversions::GetTypelessFormatFromDepthFormat(resourceDesc.format);

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
                    return desc;
                }

                D3D12_RESOURCE_DESC GetResourceDesc(const BufferDescription& resourceDesc, GpuResourceBindFlags bindFlags)
                {
                    D3D12_RESOURCE_DESC desc;

                    desc = CD3DX12_RESOURCE_DESC::Buffer(resourceDesc.size);
                    desc.Flags = TypeConversions::GetResourceFlags(bindFlags);
                    desc.Format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);
                    return desc;
                }
            }

            void ResourceImpl::ReleaseD3DObjects()
            {
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DResource_);
            }

            void ResourceImpl::Init(const Texture& resource, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint)
            {
                return Init(resource.GetDescription(), resource.GetBindFlags(), subresourcesFootprint, resource.GetName());
            }

            void ResourceImpl::Init(const TextureDescription& resourceDesc, const GpuResourceBindFlags bindFlags, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint, const U8String& name)
            {
                // TextureDesc ASSERT checks done on Texture initialization;
                ASSERT(subresourcesFootprint.size() == 0 || subresourcesFootprint.size() == resourceDesc.GetNumSubresources());

                const DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);

                D3D12_CLEAR_VALUE optimizedClearValue;
                D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
                GetOptimizedClearValue(bindFlags, format, pOptimizedClearValue);

                const D3D12_RESOURCE_DESC& desc = GetResourceDesc(resourceDesc, bindFlags);

                D3DCallMsg(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        &DefaultHeapProps,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_COMMON,
                        pOptimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())),
                    "ResourceImpl::CreateCommittedResource");

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                performInitialUpload(subresourcesFootprint);
            }

            void ResourceImpl::Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& resourceDesc, const U8String& name)
            {
                ASSERT(resource);
                // TextureDesc ASSERT checks done on Texture initialization;

                const DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);

                const D3D12_RESOURCE_DESC& desc = resource->GetDesc();
                ASSERT(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
                ASSERT(desc.Format == format);

                D3DResource_ = resource;
                D3DUtils::SetAPIName(D3DResource_.get(), name);
            }

            void ResourceImpl::Init(const Buffer& resource)
            {
                return Init(resource.GetDescription(), resource.GetBindFlags(), resource.GetName());
            }

            void ResourceImpl::Init(const BufferDescription& resourceDesc, const GpuResourceBindFlags bindFlags, const U8String& name)
            {
                ASSERT(resourceDesc.size > 0);

                const D3D12_RESOURCE_DESC& desc = GetResourceDesc(resourceDesc, bindFlags);

                D3DCallMsg(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        &DefaultHeapProps,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_COMMON,
                        nullptr,
                        IID_PPV_ARGS(D3DResource_.put())),
                    "ResourceImpl::CreateCommittedResource");

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                //    D3DCall(performInitialUpload(subresourcesFootprint));
            }

            void ResourceImpl::performInitialUpload(const std::vector<TextureSubresourceFootprint>& subresourcesFootprint)
            {
                if (subresourcesFootprint.size() == 0)
                    return;

                //subresourcesFootprint
            }
        }
    }
}