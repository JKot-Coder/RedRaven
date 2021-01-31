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

                const D3D12_HEAP_PROPERTIES* GetHeapProperties(BufferDescription::CpuAccess cpuAccess)
                {
                    switch (cpuAccess)
                    {
                    case BufferDescription::CpuAccess::None:
                        return &DefaultHeapProps;
                    case BufferDescription::CpuAccess::Write:
                        return &UploadHeapProps;
                    case BufferDescription::CpuAccess::Read:
                        return &ReadbackHeapProps;
                    default:
                        LOG_FATAL("Unsupported cpuAcess");
                    }
                }

                D3D12_RESOURCE_STATES GetDefaultResourceState(BufferDescription::CpuAccess cpuAccess)
                {
                    switch (cpuAccess)
                    {
                    case BufferDescription::CpuAccess::None:
                        return D3D12_RESOURCE_STATE_COMMON;
                    case BufferDescription::CpuAccess::Write:
                        return D3D12_RESOURCE_STATE_GENERIC_READ;
                   // case BufferDescription::CpuAccess::Read:
                   //     return &ReadbackHeapProps;
                    default:
                        LOG_FATAL("Unsupported cpuAcess");
                    }
                }

            }

            void ResourceImpl::ReleaseD3DObjects()
            {
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DResource_);
            }

            void ResourceImpl::Init(const Texture& resource, const std::shared_ptr<TextureData>& subresourceData)
            {
                return Init(resource.GetDescription(), resource.GetBindFlags(), subresourceData, resource.GetName());
            }

            void ResourceImpl::Init(const TextureDescription& resourceDesc, const GpuResourceBindFlags bindFlags, const std::shared_ptr<TextureData>& subresourceData, const U8String& name)
            {
                // TextureDesc ASSERT checks done on Texture initialization;
                ASSERT(!D3DResource_);
                ASSERT(subresourceData == nullptr || subresourceData->size() == resourceDesc.GetNumSubresources());

                const DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);

                D3D12_CLEAR_VALUE optimizedClearValue;
                D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
                GetOptimizedClearValue(bindFlags, format, pOptimizedClearValue);

                const D3D12_RESOURCE_DESC& desc = D3DUtils::GetResourceDesc(resourceDesc, bindFlags);

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        &DefaultHeapProps,
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        D3D12_RESOURCE_STATE_COMMON,
                        pOptimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())));

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                // performInitialUpload(textureData);
            }

            void ResourceImpl::Init(const ComSharedPtr<ID3D12Resource>& resource, const TextureDescription& resourceDesc, const U8String& name)
            {
                ASSERT(resource);
                ASSERT(!D3DResource_);
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
                ASSERT(!D3DResource_);
                ASSERT(resourceDesc.size > 0);

                const D3D12_RESOURCE_DESC& desc = D3DUtils::GetResourceDesc(resourceDesc, bindFlags);

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        GetHeapProperties(resourceDesc.cpuAccess),
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        GetDefaultResourceState(resourceDesc.cpuAccess),
                        nullptr,
                        IID_PPV_ARGS(D3DResource_.put())));

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                //    D3DCall(performInitialUpload(textureData));
            }

            void ResourceImpl::performInitialUpload(const std::shared_ptr<TextureData>& subresourceData)
            {
                if (subresourceData->size() == 0)
                    return;

                //textureData
            }

            void ResourceImpl::Map(uint32_t subresource, const D3D12_RANGE& range, void*& memory)
            {
                ASSERT(D3DResource_);

                D3DCall(D3DResource_->Map(subresource, &range, &memory));
            }
        }
    }
}