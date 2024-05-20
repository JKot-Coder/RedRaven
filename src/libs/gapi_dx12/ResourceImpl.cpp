#include "ResourceImpl.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/CommandListImpl.hpp" // TODO ??
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/InitialDataUploder.hpp"
#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

namespace RR
{
    namespace GAPI::DX12
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

            GpuResourceFootprint::SubresourceFootprint getSubresourceFootprint(const GpuResourceDescription& resourceDesc,
                                                                               const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout,
                                                                               UINT numRows,
                                                                               UINT64 rowSizeInBytes)
            {

                std::ignore = resourceDesc; // TODO;
                const auto rowPitch = layout.Footprint.RowPitch;
                const auto depthPitch = numRows * rowPitch;

                return GpuResourceFootprint::SubresourceFootprint(
                    layout.Offset,
                    layout.Footprint.Width,
                    layout.Footprint.Height,
                    layout.Footprint.Depth,
                    numRows, rowSizeInBytes, rowPitch, depthPitch);
            }
        }

        ResourceImpl::~ResourceImpl()
        {
            ResourceReleaseContext::DeferredD3DResourceRelease(D3DResource_, allocation_);
        }

        void ResourceImpl::DestroyImmediatly()
        {
            D3DResource_ = nullptr;
            if (allocation_)
                allocation_->Release();

            allocation_ = nullptr;
        }

        void ResourceImpl::Init(const std::shared_ptr<GpuResource>& resource)
        {
            ASSERT(resource);

            const auto& desc = resource->GetDescription();
            const auto usage = desc.usage;
            Init(desc, resource->GetName());

            // TODO UPDATE THIS CODE
            const auto& initialData = resource->GetInitialData();
            ASSERT_MSG(!initialData || (usage == GpuResourceUsage::Default) || (usage == GpuResourceUsage::Upload), "Initial resource data can only be applied to a resource with 'Default' or 'Upload' usage.");
            if (initialData)
            {
                if (usage == GpuResourceUsage::Default)
                {
                    auto& initilDataUploader = InitialDataUploder::Instance();
                    initilDataUploader.DefferedUpload(*this, initialData);
                }

                if (usage == GpuResourceUsage::Upload)
                {
                    const auto dataPointer = Map();

                    UINT64 intermediateSize;
                    const auto& device = DeviceContext::GetDevice();

                    D3D12_RESOURCE_DESC d3dResourceDesc = D3DUtils::GetResourceDesc(desc);
                    device->GetCopyableFootprints(&d3dResourceDesc, 0, desc.GetNumSubresources(), 0, nullptr, nullptr, nullptr, &intermediateSize);

                    // TODO TON OF ASSERTS AND LOGICK HERE
                    memcpy(dataPointer, initialData->Data(), intermediateSize);

                    Unmap();
                }
            }

            if (initialData)
                resource->ResetInitialData();
        }

        void ResourceImpl::Init(const GpuResourceDescription& resourceDesc, const U8String& name)
        {
            // TextureDesc ASSERT checks done on Texture initialization;
            ASSERT(!D3DResource_);

            const bool isCPUAccessible = (resourceDesc.usage == GpuResourceUsage::Readback || resourceDesc.usage == GpuResourceUsage::Upload);

            D3D12_RESOURCE_DESC d3dResourceDesc = D3DUtils::GetResourceDesc(resourceDesc);

            const auto& device = DeviceContext::GetDevice();

            UINT64 intermediateSize;
            device->GetCopyableFootprints(&d3dResourceDesc, 0, resourceDesc.GetNumSubresources(), 0, nullptr, nullptr, nullptr, &intermediateSize);

            D3D12_CLEAR_VALUE optimizedClearValue;
            D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr;

            if ((resourceDesc.dimension != GpuResourceDimension::Buffer) &&
                IsSet(resourceDesc.bindFlags, GpuResourceBindFlags::RenderTarget))
            {
                const DXGI_FORMAT format = D3DUtils::GetDxgiResourceFormat(resourceDesc.texture.format);

                pOptimizedClearValue = &optimizedClearValue;
                getOptimizedClearValue(resourceDesc.bindFlags, format, pOptimizedClearValue);
            }

            if (isCPUAccessible && resourceDesc.IsTexture())
            {
                // Dx12 don't allow textures for Upload and Readback resorces.
                d3dResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(intermediateSize);
            };

            defaultState_ = getDefaultResourceState(resourceDesc.usage);

            D3DCall(
                DeviceContext::GetDevice()->CreateCommittedResource(
                    getHeapProperties(resourceDesc.usage),
                    D3D12_HEAP_FLAG_NONE,
                    &d3dResourceDesc,
                    defaultState_,
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
            defaultState_ = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
            D3DUtils::SetAPIName(D3DResource_.get(), name);
        }

        GpuResourceFootprint ResourceImpl::GetFootprint(const GpuResourceDescription& resourceDesc)
        {
            const auto& device = DeviceContext::GetDevice();
            const auto numSubresources = resourceDesc.GetNumSubresources();

            D3D12_RESOURCE_DESC d3d12Desc = D3DUtils::GetResourceDesc(resourceDesc);

            UINT64 intermediateSize;
            device->GetCopyableFootprints(&d3d12Desc, 0, numSubresources, 0, nullptr, nullptr, nullptr, &intermediateSize);

            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
            std::vector<UINT> numRowsVector(numSubresources);
            std::vector<UINT64> rowSizeInBytesVector(numSubresources);
            device->GetCopyableFootprints(&d3d12Desc, 0, numSubresources, 0, layouts.data(), numRowsVector.data(), rowSizeInBytesVector.data(), nullptr);

            GpuResourceFootprint footprint = { std::vector<GpuResourceFootprint::SubresourceFootprint>(numSubresources), intermediateSize };
            for (uint32_t index = 0; index < numSubresources; index++)
            {
                const auto& layout = layouts[index];
                const auto numRows = numRowsVector[index];
                const auto rowSizeInBytes = rowSizeInBytesVector[index];

                footprint.subresourceFootprints[index] = getSubresourceFootprint(resourceDesc, layout, numRows, rowSizeInBytes);
            }

            return footprint;
        }

        std::vector<GpuResourceFootprint::SubresourceFootprint> ResourceImpl::GetSubresourceFootprints(const GpuResourceDescription& resourceDesc) const
        {
            const auto& device = DeviceContext::GetDevice();

            const auto numSubresources = resourceDesc.GetNumSubresources();

            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
            std::vector<UINT> numRowsVector(numSubresources);
            std::vector<UINT64> rowSizeInBytesVector(numSubresources);

            D3D12_RESOURCE_DESC d3d12Desc = D3DUtils::GetResourceDesc(resourceDesc);
            device->GetCopyableFootprints(&d3d12Desc, 0, numSubresources, 0, layouts.data(), numRowsVector.data(), rowSizeInBytesVector.data(), nullptr);

            std::vector<GpuResourceFootprint::SubresourceFootprint> subresourceFootprints(numSubresources);
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