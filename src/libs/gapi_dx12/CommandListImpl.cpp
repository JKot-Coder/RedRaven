#include "CommandListImpl.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/MemoryAllocation.hpp"

#include "gapi_dx12/CpuResourceDataAllocator.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                void CheckIsCopyAllowed(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest)
                {
                    ASSERT(source);
                    ASSERT(dest);

                    // Allow copy (gpu->gpu || cpuWrite->gpu)
                    ASSERT(source->GetCpuAccess() == GpuResourceCpuAccess::Write ||
                           source->GetCpuAccess() == GpuResourceCpuAccess::None);
                    ASSERT(dest->GetCpuAccess() == GpuResourceCpuAccess::None);
                }
            }

            void CommandListImpl::CommandAllocatorsPool::createAllocator(
                const U8String& name,
                const uint32_t index,
                ComSharedPtr<ID3D12CommandAllocator>& allocator) const
            {
                ASSERT(!allocator);

                D3DCall(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())));

                D3DUtils::SetAPIName(allocator.get(), name, index);
            }

            CommandListImpl::CommandAllocatorsPool::~CommandAllocatorsPool()
            {
            }

            void CommandListImpl::CommandAllocatorsPool::Init(
                D3D12_COMMAND_LIST_TYPE type,
                const U8String& name)
            {
                type_ = type;
                fence_ = std::make_unique<FenceImpl>();
                fence_->Init(name);

                for (uint32_t index = 0; index < allocators_.size(); index++)
                {
                    auto& allocatorData = allocators_[index];
                    createAllocator(name, index, allocatorData.allocator);
                    allocatorData.cpuFenceValue = 0;
                }
            }

            void CommandListImpl::CommandAllocatorsPool::ReleaseD3DObjects()
            {
                for (auto& allocatorData : allocators_)
                    ResourceReleaseContext::DeferredD3DResourceRelease(allocatorData.allocator);
            }

            const ComSharedPtr<ID3D12CommandAllocator>& CommandListImpl::CommandAllocatorsPool::GetNextAllocator()
            {
                auto& data = allocators_[ringBufferIndex_];

                auto gpu = fence_->GetGpuValue();

                ASSERT(data.cpuFenceValue < fence_->GetGpuValue());
                data.cpuFenceValue = fence_->GetCpuValue();
                data.allocator->Reset();

                return data.allocator;
            }

            void CommandListImpl::CommandAllocatorsPool::ResetAfterSubmit(CommandQueueImpl& commandQueue)
            {
                ringBufferIndex_ = (++ringBufferIndex_ % AllocatorsCount);
                return fence_->Signal(commandQueue);
            }

            CommandListImpl::CommandListImpl(const CommandListType commandListType)
            {
                switch (commandListType)
                {
                case CommandListType::Graphics:
                    type_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case CommandListType::Compute:
                    type_ = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case CommandListType::Copy:
                    type_ = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
                default:
                    ASSERT_MSG(false, "Unsuported command list type");
                }
            }

            void CommandListImpl::ReleaseD3DObjects()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DCommandList_);
                commandAllocatorsPool_.ReleaseD3DObjects();
            }

            void CommandListImpl::Init(const U8String& name)
            {
                ASSERT(!D3DCommandList_);

                commandAllocatorsPool_.Init(type_, name);
                const auto allocator = commandAllocatorsPool_.GetNextAllocator();

                D3DCall(DeviceContext::GetDevice()->CreateCommandList(0, type_, allocator.get(), nullptr, IID_PPV_ARGS(D3DCommandList_.put())));

                D3DUtils::SetAPIName(D3DCommandList_.get(), name);
            }

            void CommandListImpl::ResetAfterSubmit(CommandQueueImpl& commandQueue)
            {
                ASSERT(D3DCommandList_);

                commandAllocatorsPool_.ResetAfterSubmit(commandQueue);
                const auto& allocator = commandAllocatorsPool_.GetNextAllocator();
                D3DCall(D3DCommandList_->Reset(allocator.get(), nullptr));
            }

            // ---------------------------------------------------------------------------------------------
            // Copy command list
            // ---------------------------------------------------------------------------------------------

            void CommandListImpl::CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest)
            {
                CheckIsCopyAllowed(source, dest);

                const auto sourceImpl = source->GetPrivateImpl<ResourceImpl>();
                ASSERT(sourceImpl);

                const auto destImpl = dest->GetPrivateImpl<ResourceImpl>();
                ASSERT(destImpl);

                const auto& sourceDesc = source->GetDescription();
                const auto& destDesc = dest->GetDescription();

                // Actually we can copy textures with different format with restrictions. So reconsider this assert
                ASSERT(sourceDesc == destDesc);

                /* // TODO ????????????
                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(sourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
                D3DCommandList_->ResourceBarrier(1, &barrier);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(destImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
                D3DCommandList_->ResourceBarrier(1, &barrier);*/

                D3DCommandList_->CopyResource(destImpl->GetD3DObject().get(), sourceImpl->GetD3DObject().get());

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(destImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandListImpl::CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                                   const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes)
            {
                CheckIsCopyAllowed(sourceBuffer, destBuffer);

                const auto sourceImpl = sourceBuffer->GetPrivateImpl<ResourceImpl>();
                ASSERT(sourceImpl);

                const auto destImpl = destBuffer->GetPrivateImpl<ResourceImpl>();
                ASSERT(destImpl);

                D3DCommandList_->CopyBufferRegion(destImpl->GetD3DObject().get(), destOffset, sourceImpl->GetD3DObject().get(), sourceOffset, numBytes);

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(destImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(sourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandListImpl::CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                         const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx)
            {
                CheckIsCopyAllowed(sourceTexture, destTexture);

                const auto sourceImpl = sourceTexture->GetPrivateImpl<ResourceImpl>();
                ASSERT(sourceImpl);

                const auto destImpl = destTexture->GetPrivateImpl<ResourceImpl>();
                ASSERT(destImpl);

                CD3DX12_TEXTURE_COPY_LOCATION dst(destImpl->GetD3DObject().get(), destSubresourceIdx);
                CD3DX12_TEXTURE_COPY_LOCATION src(sourceImpl->GetD3DObject().get(), sourceSubresourceIdx);
                D3DCommandList_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(destImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandListImpl::CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                               const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint)
            {
                CheckIsCopyAllowed(sourceTexture, destTexture);

                const auto sourceImpl = sourceTexture->GetPrivateImpl<ResourceImpl>();
                ASSERT(sourceImpl);

                const auto destImpl = destTexture->GetPrivateImpl<ResourceImpl>();
                ASSERT(destImpl);

                const D3D12_BOX box = {
                    sourceBox.left,
                    sourceBox.top,
                    sourceBox.front,
                    sourceBox.left + sourceBox.width,
                    sourceBox.top + sourceBox.height,
                    sourceBox.front + sourceBox.depth,
                };

                CD3DX12_TEXTURE_COPY_LOCATION src(sourceImpl->GetD3DObject().get(), sourceSubresourceIdx);
                CD3DX12_TEXTURE_COPY_LOCATION dst(destImpl->GetD3DObject().get(), destSubresourceIdx);
                D3DCommandList_->CopyTextureRegion(&dst, destPoint.x, destPoint.y, destPoint.z, &src, &box);

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(destImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(sourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandListImpl::copyIntermediate(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData, bool readback) const
            {
                ASSERT(resource);
                ASSERT(resourceData);
                ASSERT(resource->GetDescription().GetNumSubresources() == resourceData->GetNumSubresources() - resourceData->GetFirstSubresource());
                // Todo add copy checks and move up to rendercontext;

                const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourceImpl);

                const auto d3dResource = resourceImpl->GetD3DObject();
                ASSERT(d3dResource);

                const auto allocation = resourceData->GetAllocation();
                ASSERT(allocation);

                const auto isTextureResource = resource->IsTexture();

                const auto& device = DeviceContext::GetDevice();
                auto desc = d3dResource->GetDesc();

                static_assert(static_cast<int>(MemoryAllocationType::Count) == 3);
                ASSERT(
                    ((allocation->GetMemoryType() == MemoryAllocationType::Upload ||
                      allocation->GetMemoryType() == MemoryAllocationType::CpuReadWrite) &&
                     readback == false) ||
                    (allocation->GetMemoryType() == MemoryAllocationType::Readback && readback == true));

                const bool cpuReadWriteResourceData = allocation->GetMemoryType() == MemoryAllocationType::CpuReadWrite;

                ComSharedPtr<ID3D12Resource> intermediateResource;
                size_t intermediateDataOffset;
                std::shared_ptr<CpuResourceData> CpuResourceData;

                HeapAllocation* gpuHeapAlloc = nullptr;

                if (cpuReadWriteResourceData)
                {
                    // Alloc intermediate resource in upload/readback heap.
                    auto memoryType = readback ? MemoryAllocationType::Readback : MemoryAllocationType::Upload;

                    CpuResourceData = CpuResourceDataAllocator::Alloc(
                        resourceData->GetResourceDescription(),
                        memoryType,
                        resourceData->GetFirstSubresource(),
                        resourceData->GetNumSubresources());

                    if (!readback)
                        CpuResourceData->CopyDataFrom(resourceData);

                    gpuHeapAlloc = CpuResourceData->GetAllocation()->GetPrivateImpl<HeapAllocation>();
                }
                else
                {
                    gpuHeapAlloc = allocation->GetPrivateImpl<HeapAllocation>();
                }

                intermediateDataOffset = gpuHeapAlloc->GetOffset();
                intermediateResource = gpuHeapAlloc->GetD3DResouce();

                const auto firstResource = resourceData->GetFirstSubresource();
                const auto numSubresources = resourceData->GetNumSubresources();
                UINT64 itermediateSize;
                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
                std::vector<UINT> numRowsVector(numSubresources);
                std::vector<UINT64> rowSizeInBytesVector(numSubresources);
                device->GetCopyableFootprints(&desc, firstResource, numSubresources, intermediateDataOffset, &layouts[0], &numRowsVector[0], &rowSizeInBytesVector[0], &itermediateSize);
                ASSERT(allocation->GetSize() == itermediateSize);

                for (uint32_t index = 0; index < resourceData->GetNumSubresources(); index++)
                {
                    const auto subresourceIndex = firstResource + index;

                    const auto& footprint = resourceData->GetSubresourceFootprints()[index];
                    const auto& layout = layouts[index];
                    const auto numRows = numRowsVector[index];
                    const auto rowSizeInBytes = rowSizeInBytesVector[index];
                    const auto rowPitch = layout.Footprint.RowPitch;
                    const auto depthPitch = numRows * rowPitch;

                    ASSERT(footprint.rowSizeInBytes == rowSizeInBytes);
                    ASSERT(footprint.depthPitch == depthPitch);
                    ASSERT(footprint.rowPitch == layout.Footprint.RowPitch);

                    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3dResource.get(), D3D12_RESOURCE_STATE_COMMON, readback ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST);
                    D3DCommandList_->ResourceBarrier(1, &barrier);

                    if (isTextureResource)
                    {
                        CD3DX12_TEXTURE_COPY_LOCATION resourceLocation(d3dResource.get(), subresourceIndex);
                        CD3DX12_TEXTURE_COPY_LOCATION intermediateLocation(intermediateResource.get(), layout);

                        const auto src = readback ? &resourceLocation : &intermediateLocation;
                        const auto dst = readback ? &intermediateLocation : &resourceLocation;

                        D3DCommandList_->CopyTextureRegion(dst, 0, 0, 0, src, nullptr);
                    }
                    else
                    {
                        const auto src = readback ? d3dResource.get() : intermediateResource.get();
                        const auto dst = readback ? intermediateResource.get() : d3dResource.get();

                        D3DCommandList_->CopyBufferRegion(dst, 0, src, 0, rowSizeInBytes);
                    }

                    barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3dResource.get(), readback ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
                    D3DCommandList_->ResourceBarrier(1, &barrier);
                }
            }

            void CommandListImpl::UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData)
            {
                copyIntermediate(resource, resourceData, false);
            }

            void CommandListImpl::ReadbackGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData)
            {
                copyIntermediate(resource, resourceData, true);
            }

            // ---------------------------------------------------------------------------------------------
            // Compute command list
            // ---------------------------------------------------------------------------------------------

            void CommandListImpl::ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue)
            {
                ASSERT(unorderedAcessView);
            }

            void CommandListImpl::ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue)
            {
                ASSERT(unorderedAcessView);
            }

            // ---------------------------------------------------------------------------------------------
            // Graphics command list
            // ---------------------------------------------------------------------------------------------

            void CommandListImpl::ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color)
            {
                ASSERT(renderTargetView);
                ASSERT(D3DCommandList_);

                const auto allocation = renderTargetView->GetPrivateImpl<DescriptorHeap::Allocation>();
                ASSERT(allocation);

                const auto& resource = renderTargetView->GetGpuResource().lock();
                ASSERT(resource);
                ASSERT(resource->IsTexture());

                const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourceImpl);

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
                D3DCommandList_->ResourceBarrier(1, &barrier);

                D3DCommandList_->ClearRenderTargetView(allocation->GetCPUHandle(), &color.x, 0, nullptr);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandListImpl::Close()
            {
                D3DCall(D3DCommandList_->Close());
            }
        };
    }
}