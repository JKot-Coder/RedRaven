#include "ResourceImpl.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
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

                const D3D12_HEAP_PROPERTIES* getHeapProperties(GpuResourceUsage usage)
                {
                    switch (usage)
                    {
                        case GpuResourceUsage::Default: return &DefaultHeapProps;
                        case GpuResourceUsage::Upload: return &UploadHeapProps;
                        case GpuResourceUsage::Readback: return &ReadbackHeapProps;
                        default: LOG_FATAL("Unsupported cpuAcess");
                    }

                    return nullptr;
                }

                D3D12_RESOURCE_STATES getDefaultResourceState(GpuResourceUsage usage)
                {
                    switch (usage)
                    {
                        case GpuResourceUsage::Default: return D3D12_RESOURCE_STATE_COMMON;
                        case GpuResourceUsage::Upload: return D3D12_RESOURCE_STATE_GENERIC_READ;
                        case GpuResourceUsage::Readback: return D3D12_RESOURCE_STATE_COPY_DEST;
                        default: LOG_FATAL("Unsupported cpuAcess");
                    }

                    return D3D12_RESOURCE_STATE_COMMON;
                }
            }

            ResourceImpl::~ResourceImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DResource_, allocation_);
            }

            void ResourceImpl::Init(const Texture& resource)
            {
                return Init(resource.GetDescription(), resource.GetUsage(), resource.GetName());
            }

            void ResourceImpl::Init(
                const GpuResourceDescription& resourceDesc,
                GpuResourceUsage usage,
                const U8String& name)
            {
                // TextureDesc ASSERT checks done on Texture initialization;
                ASSERT(!D3DResource_);

                const bool isCPUAccessible = (usage == GpuResourceUsage::Readback || usage == GpuResourceUsage::Upload);
                const bool isTexutre = resourceDesc.GetDimension() != GpuResourceDimension::Buffer;

                D3D12_RESOURCE_DESC d3dResourceDesc = D3DUtils::GetResourceDesc(resourceDesc);

                const auto& device = DeviceContext::GetDevice();

                UINT64 intermediateSize;
                device->GetCopyableFootprints(&d3dResourceDesc, 0, resourceDesc.GetNumSubresources(), 0, nullptr, nullptr, nullptr, &intermediateSize);

                const DXGI_FORMAT format = D3DUtils::GetDxgiResourceFormat(resourceDesc.GetFormat());

                D3D12_CLEAR_VALUE optimizedClearValue;
                D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
                getOptimizedClearValue(resourceDesc.GetBindFlags(), format, pOptimizedClearValue);

                if (isCPUAccessible && isTexutre)
                {
                    // Dx12 don't allow textures for Upload and Readback resorces.
                    d3dResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(intermediateSize);
                };

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        getHeapProperties(usage),
                        D3D12_HEAP_FLAG_NONE,
                        &d3dResourceDesc,
                        getDefaultResourceState(usage),
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
                return Init(resource.GetDescription(), resource.GetUsage(), resource.GetName());
            }

            void ResourceImpl::Map(uint32_t subresource, const D3D12_RANGE& readRange, void*& memory)
            {
                ASSERT(D3DResource_);
                // todo subresource readRange asserts
                // todo mapped unmaped protection

                D3DCall(D3DResource_->Map(subresource, &readRange, &memory));
            }

            void ResourceImpl::Unmap(uint32_t subresource, const D3D12_RANGE& writtenRange)
            {
                ASSERT(D3DResource_);
                // todo subresource readRange asserts

                D3DResource_->Unmap(subresource, &writtenRange);
            }
        }
    }
}