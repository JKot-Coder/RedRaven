#include "CommandListImpl.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/FramebufferImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

#include "math/VectorMath.hpp"

namespace RR::GAPI::DX12
{
    namespace
    {
        bool isCopyAllowed(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest)
        {
            ASSERT(source);
            ASSERT(dest);

            const auto sourceUsage = source->GetDescription().usage;
            const auto destUsage = dest->GetDescription().usage;

            return (sourceUsage == GpuResourceUsage::Default && destUsage == GpuResourceUsage::Default) ||
                   (sourceUsage == GpuResourceUsage::Upload && destUsage == GpuResourceUsage::Default) ||
                   (sourceUsage == GpuResourceUsage::Default && destUsage == GpuResourceUsage::Readback);
        }

        D3D12_HEAP_TYPE GetHeapType(const ComSharedPtr<ID3D12Resource>& resource)
        {
            ASSERT(resource);

            D3D12_HEAP_PROPERTIES properties;
            resource->GetHeapProperties(&properties, nullptr);

            return properties.Type;
        }

        bool isCopyAllowed(D3D12_HEAP_TYPE sourceHeapType, D3D12_HEAP_TYPE destHeapType)
        {
            return (sourceHeapType == D3D12_HEAP_TYPE_DEFAULT && destHeapType == D3D12_HEAP_TYPE_DEFAULT) ||
                   (sourceHeapType == D3D12_HEAP_TYPE_UPLOAD && destHeapType == D3D12_HEAP_TYPE_DEFAULT) ||
                   (sourceHeapType == D3D12_HEAP_TYPE_DEFAULT && destHeapType == D3D12_HEAP_TYPE_READBACK);
        }
    }

    ComSharedPtr<ID3D12CommandAllocator> CommandListImpl::CommandAllocatorsPool::createAllocator() const
    {
        ComSharedPtr<ID3D12CommandAllocator> allocator;

        D3DCall(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())));
        D3DUtils::SetAPIName(allocator.get(), name_, allocators_.size() + 1);

        return allocator;
    }

    CommandListImpl::CommandAllocatorsPool::~CommandAllocatorsPool()
    {
        while (!allocators_.empty())
        {
            auto& allocator = allocators_.front();
            ResourceReleaseContext::DeferredD3DResourceRelease(allocator.first);

            allocators_.pop();
        }
    }

    void CommandListImpl::CommandAllocatorsPool::Init(
        D3D12_COMMAND_LIST_TYPE type,
        const U8String& name)
    {
        type_ = type;
        name_ = name;
        fence_ = std::make_unique<FenceImpl>();
        fence_->Init(name);

        for (uint32_t index = 0; index < InitialAllocatorsCount; index++)
            allocators_.emplace(createAllocator(), 0);
    }

    ComSharedPtr<ID3D12CommandAllocator> CommandListImpl::CommandAllocatorsPool::GetNextAllocator()
    {
        auto allocator = allocators_.front();

        if (allocator.second >= fence_->GetGpuValue())
        {
            // The oldest allocator doesn't executed yet, create new one
            return allocators_.emplace(createAllocator(), fence_->GetCpuValue()).first;
        }

        // Reuse old one
        allocator.second = fence_->GetCpuValue();
        allocator.first->Reset();
        allocators_.pop();
        allocators_.push(allocator); // Place in the end

        return allocator.first;
    }

    void CommandListImpl::CommandAllocatorsPool::ResetAfterSubmit(CommandQueueImpl& commandQueue)
    {
        return fence_->Signal(commandQueue);
    }

    CommandListImpl::CommandListImpl(const CommandListType commandListType)
    {
        switch (commandListType)
        {
            case CommandListType::Graphics: type_ = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
            case CommandListType::Compute: type_ = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
            case CommandListType::Copy: type_ = D3D12_COMMAND_LIST_TYPE_COPY; break;
            default: ASSERT_MSG(false, "Unsuported command list type"); type_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    CommandListImpl::~CommandListImpl()
    {
        ResourceReleaseContext::DeferredD3DResourceRelease(D3DCommandList_);
    }

    void CommandListImpl::Init(const U8String& name, int32_t index)
    {
        ASSERT(!D3DCommandList_);

        commandAllocatorsPool_.Init(type_, name);
        const auto allocator = commandAllocatorsPool_.GetNextAllocator();

        D3DCall(DeviceContext::GetDevice()->CreateCommandList(0, type_, allocator.get(), nullptr, IID_PPV_ARGS(D3DCommandList_.put())));
        D3DUtils::SetAPIName(D3DCommandList_.get(), name, index);
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

    void CommandListImpl::CopyGpuResource(const ResourceImpl& source, const ResourceImpl& dest)
    {
        ASSERT(D3DCommandList_)

        // Todo add copy checks and move up to rendercontext;

        const auto d3dSource = source.GetD3DObject();
        ASSERT(d3dSource);

        const auto d3dDest = dest.GetD3DObject();
        ASSERT(d3dDest);

        const auto& d3dDescSource = d3dSource->GetDesc();
        const auto& d3dDescDest = d3dDest->GetDesc();

        const auto sourceHeapType = GetHeapType(d3dSource);
        const auto destHeapType = GetHeapType(d3dDest);

        ASSERT(isCopyAllowed(sourceHeapType, destHeapType));

        if (d3dDescSource.Dimension == d3dDescDest.Dimension)
        {
            // Just copy resoruces with same dimension
            D3DCommandList_->CopyResource(d3dDest.get(), d3dSource.get());

            if (dest.GetDefaultResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
            {
                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3dDest.get(), D3D12_RESOURCE_STATE_COPY_DEST, dest.GetDefaultResourceState());
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }
        }
        else
        {
            // Possible copy intermediate buffer data -> GPU or GPU -> intermediate buffer data

            enum class CopyDirection
            {
                Unknown,
                Upload,
                Reaback
            } copyDirection = CopyDirection::Unknown;

            if (sourceHeapType == D3D12_HEAP_TYPE_UPLOAD &&
                destHeapType == D3D12_HEAP_TYPE_DEFAULT)
                copyDirection = CopyDirection::Upload;

            if (sourceHeapType == D3D12_HEAP_TYPE_DEFAULT &&
                destHeapType == D3D12_HEAP_TYPE_READBACK)
                copyDirection = CopyDirection::Reaback;

            ASSERT(copyDirection != CopyDirection::Unknown)
            if (copyDirection == CopyDirection::Unknown)
                return;

            const auto& gpuResource = (copyDirection == CopyDirection::Upload) ? d3dDest : d3dSource;
            const auto& cpuResource = (copyDirection == CopyDirection::Upload) ? d3dSource : d3dDest;
            const auto& gpuResourceDesc = (copyDirection == CopyDirection::Upload) ? d3dDescDest : d3dDescSource;
            const auto& cpuResourceDesc = (copyDirection == CopyDirection::Upload) ? d3dDescSource : d3dDescDest;

            uint32_t numSubresources = D3DUtils::GetSubresourcesCount(gpuResourceDesc);
            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);

            const auto& device = DeviceContext::GetDevice();

            UINT64 itermediateSize;
            device->GetCopyableFootprints(&gpuResourceDesc, 0, numSubresources, 0, &layouts[0], nullptr, nullptr, &itermediateSize);

            ASSERT(gpuResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER);
            ASSERT(cpuResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
            ASSERT(cpuResourceDesc.Width == itermediateSize);

            for (uint32_t index = 0; index < numSubresources; index++)
            {
                const auto& layout = layouts[index];

                //    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3dResource.get(), D3D12_RESOURCE_STATE_COMMON, readback ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST);
                //  D3DCommandList_->ResourceBarrier(1, &barrier);

                CD3DX12_TEXTURE_COPY_LOCATION gpuResourceLocation(gpuResource.get(), index);
                CD3DX12_TEXTURE_COPY_LOCATION cpuResoruceLocation(cpuResource.get(), layout);

                const auto src = (copyDirection == CopyDirection::Upload) ? &cpuResoruceLocation : &gpuResourceLocation;
                const auto dst = (copyDirection == CopyDirection::Upload) ? &gpuResourceLocation : &cpuResoruceLocation;

                D3DCommandList_->CopyTextureRegion(dst, 0, 0, 0, src, nullptr);

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(gpuResource.get(), (copyDirection == CopyDirection::Upload) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }
        }
    }

    void CommandListImpl::CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest)
    {
        ASSERT(source);
        ASSERT(dest);

        ASSERT(isCopyAllowed(source, dest));

        const auto sourceImpl = source->GetPrivateImpl<ResourceImpl>();
        ASSERT(sourceImpl);

        const auto destImpl = dest->GetPrivateImpl<ResourceImpl>();
        ASSERT(destImpl);

        CopyGpuResource(*sourceImpl, *destImpl);
    }

    void CommandListImpl::CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                           const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes)
    {
        ASSERT(sourceBuffer);
        ASSERT(destBuffer);

        ASSERT(isCopyAllowed(sourceBuffer, destBuffer));

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
        ASSERT(sourceTexture);
        ASSERT(destTexture);

        ASSERT(isCopyAllowed(sourceTexture, destTexture));

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
        ASSERT(sourceTexture);
        ASSERT(destTexture);

        ASSERT(isCopyAllowed(sourceTexture, destTexture));

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

    void CommandListImpl::UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<IDataBuffer>& resourceData)
    {
        ASSERT(resource);
        ASSERT(resourceData);
        // const bool readback = false;
        // Todo add copy checks and move up to rendercontext;

        // const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();
        // ASSERT(resourceImpl);

        // const auto d3dResource = resourceImpl->GetD3DObject();
        // ASSERT(d3dResource);

        // const auto isTextureResource = resource->GetDescription().IsTexture();

        // const auto& device = DeviceContext::GetDevice();
        // auto desc = d3dResource->GetDesc();

        //  static_assert(static_cast<int>(MemoryAllocationType::Count) == 3);
        /* ASSERT(
                    ((allocation->GetMemoryType() == MemoryAllocationType::Upload ||
                      allocation->GetMemoryType() == MemoryAllocationType::CpuReadWrite) &&
                     readback == false) ||
                    (allocation->GetMemoryType() == MemoryAllocationType::Readback && readback == true));
                    */

        //  const bool cpuReadWriteResourceData = allocation->GetMemoryType() == MemoryAllocationType::CpuReadWrite;
        /*
                ComSharedPtr<ID3D12Resource> intermediateResource;
                size_t intermediateDataOffset;
                std::shared_ptr<CpuResourceData> CpuResourceData;

                HeapAllocation* gpuHeapAlloc = nullptr;

                if (cpuReadWriteResourceData)
                {
                    // Allocate intermediate resource in upload/readback heap.
                    auto memoryType = readback ? MemoryAllocationType::Readback : MemoryAllocationType::Upload;

                    CpuResourceData = CpuResourceDataAllocator::Allocate(
                        resource->GetDescription(),
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
                }*/
    }

    // ---------------------------------------------------------------------------------------------
    // Compute command list
    // ---------------------------------------------------------------------------------------------

    void CommandListImpl::SetFrameBuffer(const std::shared_ptr<Framebuffer>& framebuffer)
    {
        ASSERT(framebuffer);

        const auto frameBufferImpl = framebuffer->GetPrivateImpl<FramebufferImpl>();
        ASSERT(frameBufferImpl);

        D3DCommandList_->OMSetRenderTargets(frameBufferImpl->GetRTVDescriptiorsCount(), frameBufferImpl->GetRTVDescriptiors(), false, frameBufferImpl->GetDSVDescriptor());
    }

    void CommandListImpl::SetIndexBuffer(const std::shared_ptr<Buffer>& buffer, size_t offset)
    {
        ASSERT(buffer);
        ASSERT(buffer->GetDescription().IsIndex());
        ASSERT(offset < buffer->GetDescription().buffer.size);

        const auto bufferImpl = buffer->GetPrivateImpl<ResourceImpl>();
        ASSERT(bufferImpl);

        D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
        indexBufferView.BufferLocation = bufferImpl->GetD3DObject()->GetGPUVirtualAddress() + offset;
        indexBufferView.Format = D3DUtils::GetDxgiResourceFormat(buffer->GetDescription().GetIndexBufferFormat());
        indexBufferView.SizeInBytes = buffer->GetDescription().buffer.size - offset;

        D3DCommandList_->IASetIndexBuffer(&indexBufferView);
    }

    void CommandListImpl::ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue)
    {
        ASSERT(unorderedAcessView);

        const auto& resource = unorderedAcessView->GetGpuResource().lock();
        ASSERT(resource);

        const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();
        ASSERT(resourceImpl);

        const auto resourceViewImpl = unorderedAcessView->GetPrivateImpl<DescriptorHeap::Descriptor>();
        ASSERT(resourceViewImpl);

        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        D3DCommandList_->ResourceBarrier(1, &barrier);

        D3DCommandList_->ClearUnorderedAccessViewUint(resourceViewImpl->GetGPUHandle(), resourceViewImpl->GetCPUHandle(), resourceImpl->GetD3DObject().get(), &clearValue.x, 0, nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
        D3DCommandList_->ResourceBarrier(1, &barrier);
    }

    void CommandListImpl::ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue)
    {
        ASSERT(unorderedAcessView);

        const auto& resource = unorderedAcessView->GetGpuResource().lock();
        ASSERT(resource);

        const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();
        ASSERT(resourceImpl);

        const auto resourceViewImpl = unorderedAcessView->GetPrivateImpl<DescriptorHeap::Descriptor>();
        ASSERT(resourceViewImpl);

        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        D3DCommandList_->ResourceBarrier(1, &barrier);

        D3DCommandList_->ClearUnorderedAccessViewFloat(resourceViewImpl->GetGPUHandle(), resourceViewImpl->GetCPUHandle(), resourceImpl->GetD3DObject().get(), &clearValue.x, 0, nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
        D3DCommandList_->ResourceBarrier(1, &barrier);
    }

    // ---------------------------------------------------------------------------------------------
    // Graphics command list
    // ---------------------------------------------------------------------------------------------

    void CommandListImpl::ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color)
    {
        ASSERT(renderTargetView);
        ASSERT(D3DCommandList_);

        const auto allocation = renderTargetView->GetPrivateImpl<DescriptorHeap::Descriptor>();
        ASSERT(allocation);

        const auto& resource = renderTargetView->GetGpuResource().lock();
        ASSERT(resource);
        ASSERT(resource->GetDescription().IsTexture());

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

}