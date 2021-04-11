#include "CpuResourceDataAllocator.hpp"

#include "gapi/Texture.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            HeapAllocation::HeapAllocation(D3D12_HEAP_TYPE heapType, size_t size)
                : heapType_(heapType),
                  size_(size)
            {
                const auto& resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size_);

                D3D12MA::ALLOCATION_DESC allocationDesc = {};
                allocationDesc.HeapType = heapType_;

                ASSERT(heapType == D3D12_HEAP_TYPE_READBACK || heapType == D3D12_HEAP_TYPE_UPLOAD);
                const auto defaultState = (heapType == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;

                ComSharedPtr<ID3D12Resource> d3dresource;
                D3D12MA::Allocation* allocation;
                HRESULT hr = DeviceContext::GetAllocator()->CreateResource(
                    &allocationDesc,
                    &resourceDesc,
                    defaultState,
                    NULL,
                    &allocation,
                    IID_PPV_ARGS(d3dresource.put()));

                resource_ = std::make_shared<ResourceImpl>();
                resource_->Init(d3dresource, allocation, "heapAlloc");
            }

            HeapAllocation::~HeapAllocation()
            {
                if (isMapped_)
                    Unmap();

                resource_->ReleaseD3DObjects();
                resource_.reset();
            }

            void* HeapAllocation::Map() const
            {
                ASSERT(!isMapped_);
                D3D12_RANGE readRange { 0, heapType_ == D3D12_HEAP_TYPE_READBACK ? size_ : 0 };

                isMapped_ = true;

                void* mappedData;
                resource_->Map(0, readRange, mappedData);

                return mappedData;
            }

            void HeapAllocation::Unmap() const
            {
                ASSERT(isMapped_);

                D3D12_RANGE writtenRange { 0, heapType_ == D3D12_HEAP_TYPE_UPLOAD ? size_ : 0 };
                resource_->Unmap(0, writtenRange);

                isMapped_ = false;
            }

            ComSharedPtr<ID3D12Resource> HeapAllocation::GetD3DResouce() const
            {
                return resource_->GetD3DObject();
            }

            std::shared_ptr<CpuResourceData> const CpuResourceDataAllocator::Alloc(
                const GpuResourceDescription& resourceDesc,
                MemoryAllocationType memoryType,
                uint32_t firstSubresourceIndex,
                uint32_t numSubresources)
            {
                if (numSubresources == Texture::MaxPossible)
                    numSubresources = resourceDesc.GetNumSubresources();

                ASSERT(resourceDesc.GetDimension() != GpuResourceDimension::Texture2DMS);
                ASSERT(resourceDesc.GetSampleCount() == 1);
                ASSERT(firstSubresourceIndex + numSubresources <= resourceDesc.GetNumSubresources());
                D3D12_RESOURCE_DESC desc = D3DUtils::GetResourceDesc(resourceDesc);

                const auto& device = DeviceContext::GetDevice();

                UINT64 intermediateSize;
                device->GetCopyableFootprints(&desc, 0, numSubresources, 0, nullptr, nullptr, nullptr, &intermediateSize);

                const auto& allocation = std::make_shared<MemoryAllocation>(memoryType, intermediateSize);

                IMemoryAllocation* memoryAllocation;
                switch (memoryType)
                {
                case MemoryAllocationType::Upload:
                    memoryAllocation = new HeapAllocation(D3D12_HEAP_TYPE_UPLOAD, intermediateSize);
                    break;
                case MemoryAllocationType::Readback:
                    memoryAllocation = new HeapAllocation(D3D12_HEAP_TYPE_READBACK, intermediateSize);
                    break;
                case MemoryAllocationType::CpuReadWrite:
                    memoryAllocation = new CpuAllocation(intermediateSize);
                    break;
                default:
                    LOG_FATAL("Unsupported memory type");
                }

                ASSERT(memoryAllocation);
                allocation->SetPrivateImpl(memoryAllocation);

                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
                std::vector<UINT> numRowsVector(numSubresources);
                std::vector<UINT64> rowSizeInBytesVector(numSubresources);

                device->GetCopyableFootprints(&desc, firstSubresourceIndex, numSubresources, 0, &layouts[0], &numRowsVector[0], &rowSizeInBytesVector[0], nullptr);

                std::vector<CpuResourceData::SubresourceFootprint> subresourceFootprints(numSubresources);
                for (uint32_t index = 0; index < numSubresources; index++)
                {
                    const auto& layout = layouts[index];
                    const auto numRows = numRowsVector[index];
                    const auto rowSizeInBytes = rowSizeInBytesVector[index];
                    const auto rowPitch = layout.Footprint.RowPitch;
                    const auto depthPitch = numRows * rowPitch;
          
                    subresourceFootprints[index] = CpuResourceData::SubresourceFootprint(
                        layout.Offset,
                        layout.Footprint.Width,
                        layout.Footprint.Height,
                        layout.Footprint.Depth,
                        numRows, rowSizeInBytes, rowPitch, depthPitch);
                }

                return std::make_shared<CpuResourceData>(allocation, subresourceFootprints, firstSubresourceIndex);
            }
        }
    }
}