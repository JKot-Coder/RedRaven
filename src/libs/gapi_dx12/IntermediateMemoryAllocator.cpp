#include "IntermediateMemoryAllocator.hpp"

#include "gapi/Texture.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/GpuMemoryHeap.hpp"
#include "gapi_dx12/ResourceImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            std::shared_ptr<IntermediateMemory> const IntermediateMemoryAllocator::AllocateIntermediateTextureData(
                const TextureDescription& resourceDesc,
                MemoryAllocationType memoryType,
                uint32_t firstSubresourceIndex,
                uint32_t numSubresources)
            {
                if (numSubresources == Texture::MaxPossible)
                    numSubresources = resourceDesc.GetNumSubresources();

                ASSERT(firstSubresourceIndex + numSubresources <= resourceDesc.GetNumSubresources())
                D3D12_RESOURCE_DESC desc = D3DUtils::GetResourceDesc(resourceDesc, GpuResourceBindFlags::None);

                const auto& device = DeviceContext::GetDevice();

                UINT64 intermediateSize;
                device->GetCopyableFootprints(&desc, 0, numSubresources, 0, nullptr, nullptr, nullptr, &intermediateSize);

                const auto& allocation = std::make_shared<MemoryAllocation>(memoryType, intermediateSize);

                switch (memoryType)
                {
                case MemoryAllocationType::Upload:
                {
                    const auto heapAlloc = DeviceContext::GetUploadHeap()->Allocate(intermediateSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                    allocation->SetPrivateImpl(heapAlloc);
                    break;
                }
                case MemoryAllocationType::Readback:
                {
                    const auto heapAlloc = DeviceContext::GetReadbackHeap()->Allocate(intermediateSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                    allocation->SetPrivateImpl(heapAlloc);
                    break;
                }
                case MemoryAllocationType::CpuReadWrite:
                    allocation->SetPrivateImpl(new CpuAllocation(intermediateSize));
                    break;
                default:
                    LOG_FATAL("Unsupported memory type");
                }

                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
                std::vector<UINT> numRowsVector(numSubresources);
                std::vector<UINT64> rowSizeInBytesVector(numSubresources);

                device->GetCopyableFootprints(&desc, firstSubresourceIndex, numSubresources, 0, &layouts[0], &numRowsVector[0], &rowSizeInBytesVector[0], nullptr);

                std::vector<IntermediateMemory::SubresourceFootprint> subresourceFootprints(numSubresources);
                for (uint32_t index = 0; index < numSubresources; index++)
                {
                    const auto& layout = layouts[index];
                    const auto numRows = numRowsVector[index];
                    const auto rowSizeInBytes = rowSizeInBytesVector[index];
                    const auto rowPitch = layout.Footprint.RowPitch;
                    const auto depthPitch = numRows * rowPitch;

                    subresourceFootprints[index] = IntermediateMemory::SubresourceFootprint(layout.Offset, numRows, rowSizeInBytes, rowPitch, depthPitch);
                }

                return std::make_shared<IntermediateMemory>(allocation, subresourceFootprints, firstSubresourceIndex);
            }
        }
    }
}