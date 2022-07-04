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

                CpuResourceData::SubresourceFootprint getSubresourceFootprint(const GpuResourceDescription& resourceDesc,
                                                                              const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout,
                                                                              UINT numRows,
                                                                              UINT64 rowSizeInBytes)
                {
                    const auto rowPitch = layout.Footprint.RowPitch;
                    const auto depthPitch = numRows * rowPitch;

                    return CpuResourceData::SubresourceFootprint(
                        layout.Offset,
                        (resourceDesc.GetDimension() == GpuResourceDimension::Buffer) ? resourceDesc.GetNumElements() : layout.Footprint.Width,
                        layout.Footprint.Height,
                        layout.Footprint.Depth,
                        numRows, rowSizeInBytes, rowPitch, depthPitch);
                }
            }

            ResourceImpl::~ResourceImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DResource_, allocation_);
            }

            void ResourceImpl::Init(const Texture& resource)
            {
                return Init(resource.GetDescription(), resource.GetInitialData(), resource.GetUsage(), resource.GetName());
            }

            void ResourceImpl::Init(
                const GpuResourceDescription& resourceDesc,
                const IDataBuffer::SharedPtr& initialData,
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

                defaultState_ = getDefaultResourceState(usage);

                D3DCall(
                    DeviceContext::GetDevice()->CreateCommittedResource(
                        getHeapProperties(usage),
                        D3D12_HEAP_FLAG_NONE,
                        &d3dResourceDesc,
                        defaultState_,
                        pOptimizedClearValue,
                        IID_PPV_ARGS(D3DResource_.put())));

                D3DUtils::SetAPIName(D3DResource_.get(), name);

                ASSERT_MSG(!initialData || (usage != GpuResourceUsage::Default), "Initial resource data can only be applied to a resource with 'Default' usage.");
                if (initialData && (usage == GpuResourceUsage::Default))
                {
                    DeviceContext::GetInitialDataUploader(); 
                }

            }

            void ResourceImpl::Init(const ComSharedPtr<ID3D12Resource>& resource, D3D12MA::Allocation* allocation, const U8String& name)
            {
                ASSERT(resource);
                ASSERT(!D3DResource_);

                allocation_ = allocation;
                D3DResource_ = resource;
                defaultState_ = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
                D3DUtils::SetAPIName(D3DResource_.get(), name);
            }

            void ResourceImpl::Init(const Buffer& resource)
            {
                return Init(resource.GetDescription(), resource.GetInitialData(), resource.GetUsage(), resource.GetName());
            }

            std::vector<CpuResourceData::SubresourceFootprint> ResourceImpl::GetSubresourceFootprints(const GpuResourceDescription& resourceDesc) const
            {
                const auto& device = DeviceContext::GetDevice();

                const auto numSubresources = resourceDesc.GetNumSubresources();

                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
                std::vector<UINT> numRowsVector(numSubresources);
                std::vector<UINT64> rowSizeInBytesVector(numSubresources);

                D3D12_RESOURCE_DESC d3d12Desc = D3DUtils::GetResourceDesc(resourceDesc);
                device->GetCopyableFootprints(&d3d12Desc, 0, numSubresources, 0, layouts.data(), numRowsVector.data(), rowSizeInBytesVector.data(), nullptr);

                std::vector<CpuResourceData::SubresourceFootprint> subresourceFootprints(numSubresources);
                for (uint32_t index = 0; index < numSubresources; index++)
                {
                    const auto& layout = layouts[index];
                    const auto numRows = numRowsVector[index];
                    const auto rowSizeInBytes = rowSizeInBytesVector[index];

                    subresourceFootprints[index] = getSubresourceFootprint(resourceDesc, layout, numRows, rowSizeInBytes);
                }

                return subresourceFootprints;
            }

            void* ResourceImpl::Map()
            {
                ASSERT(D3DResource_);
                // todo mapped unmaped protection

                void* memory;
                D3DCall(D3DResource_->Map(0, nullptr, &memory));

                return memory;
            }

            void ResourceImpl::Unmap()
            {
                ASSERT(D3DResource_);

                D3DResource_->Unmap(0, nullptr);
            }
        }
    }
}