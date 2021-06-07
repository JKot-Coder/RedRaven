#include "ResourceImpl.hpp"

#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                void getOptimizedClearValue(GpuResourceBindFlags bindFlags, DXGI_FORMAT format, D3D12_CLEAR_VALUE*& value)
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

                const D3D12_HEAP_PROPERTIES* getHeapProperties(GpuResourceCpuAccess cpuAccess)
                {
                    switch (cpuAccess)
                    {
                    case GpuResourceCpuAccess::None:
                        return &DefaultHeapProps;
                    case GpuResourceCpuAccess::Write:
                        return &UploadHeapProps;
                    case GpuResourceCpuAccess::Read:
                        return &ReadbackHeapProps;
                    default:
                        LOG_FATAL("Unsupported cpuAcess");
                    }
                }

                D3D12_RESOURCE_STATES getDefaultResourceState(GpuResourceCpuAccess cpuAccess)
                {
                    switch (cpuAccess)
                    {
                    case GpuResourceCpuAccess::None:
                        return D3D12_RESOURCE_STATE_COMMON;
                    case GpuResourceCpuAccess::Write:
                        return D3D12_RESOURCE_STATE_GENERIC_READ;
                    case GpuResourceCpuAccess::Read:
                        return D3D12_RESOURCE_STATE_COPY_DEST;
                    default:
                        LOG_FATAL("Unsupported cpuAcess");
                    }
                }
            }

            ResourceImpl::~ResourceImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DResource_, allocation_);
            }

            void ResourceImpl::Init(const Texture& resource)
            {
                return Init(resource.GetDescription(), resource.GetCpuAccess(), resource.GetName());
            }

            void ResourceImpl::Init(
                const GpuResourceDescription& resourceDesc,
                GpuResourceCpuAccess cpuAccess,
                const U8String& name)
            {
                // TextureDesc ASSERT checks done on Texture initialization;
                ASSERT(!D3DResource_);

                const DXGI_FORMAT format = D3DUtils::GetDxgiResourceFormat(resourceDesc.GetFormat());

                D3D12_CLEAR_VALUE optimizedClearValue;
                D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
                getOptimizedClearValue(resourceDesc.GetBindFlags(), format, pOptimizedClearValue);

                const D3D12_RESOURCE_DESC& desc = D3DUtils::GetResourceDesc(resourceDesc);

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        getHeapProperties(cpuAccess),
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        getDefaultResourceState(cpuAccess),
                        pOptimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())));

                D3DUtils::SetAPIName(D3DResource_.get(), name);
            }

            void ResourceImpl::Init(const ComSharedPtr<ID3D12Resource>& resource, D3D12MA::Allocation* allocation, const U8String& name)
            {
                ASSERT(resource);
                ASSERT(!D3DResource_);

                allocation_ = allocation;
                D3DResource_ = resource;
                D3DUtils::SetAPIName(D3DResource_.get(), name);
            }

            void ResourceImpl::Init(const Buffer& resource)
            {
                return Init(resource.GetDescription(), resource.GetCpuAccess(), resource.GetName());
            }
            /*
            * //TODO buffer support
            void ResourceImpl::Init(
                const BufferDescription& resourceDesc,
                GpuResourceCpuAccess cpuAccess,
                const U8String& name)
            {
                ASSERT(!D3DResource_);
                ASSERT(resourceDesc.width > 0);

                const D3D12_RESOURCE_DESC& desc = D3DUtils::GetResourceDesc(resourceDesc);

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        GetHeapProperties(cpuAccess),
                        D3D12_HEAP_FLAG_NONE,
                        &desc,
                        GetDefaultResourceState(cpuAccess),
                        nullptr,
                        IID_PPV_ARGS(D3DResource_.put())));

                D3DUtils::SetAPIName(D3DResource_.get(), name);
            }*/

            void ResourceImpl::Map(uint32_t subresource, const D3D12_RANGE& readRange, void*& memory)
            {
                ASSERT(D3DResource_);
                // todo subresource readRange asserts

                D3DCall(D3DResource_->Map(subresource, &readRange, &memory));
            }

            void ResourceImpl::Unmap(uint32_t subresource, const D3D12_RANGE& writtenRange)
            {
                ASSERT(D3DResource_);
                // todo subresource readRange asserts

                D3DResource_->Unmap(subresource, & writtenRange);
            }
        }
    }
}