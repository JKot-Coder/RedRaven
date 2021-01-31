#include "CommandListImpl.hpp"

#include "d3dx12.h"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/MemoryAllocation.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/GpuMemoryHeap.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void CommandListImpl::CommandAllocatorsPool::createAllocator(
                const U8String& name,
                const uint32_t index,
                ComSharedPtr<ID3D12CommandAllocator>& allocator) const
            {
                ASSERT(!allocator);

                D3DCall(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())));

                D3DUtils::SetAPIName(allocator.get(), name, index);
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
                    DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(allocatorData.allocator);
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
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DCommandList_);
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

            void CommandListImpl::CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer)
            {
            }

            void CommandListImpl::CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                                   const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes)
            {
            }

            void CommandListImpl::CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture)
            {
            }

            void CommandListImpl::CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                                         const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx)
            {
            }

            void CommandListImpl::CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                               const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint)
            {
            }

            void CommandListImpl::UpdateTextureData(const std::shared_ptr<Texture>& texture, const std::shared_ptr<TextureData>& textureData)
            {
                ASSERT(texture);
                ASSERT(textureData);
                ASSERT(texture->GetDescription().GetNumSubresources() == textureData->size());

                for (const auto& subresourceData : *textureData.get())
                    UpdateSubresourceData(texture, subresourceData);
            }

            void CommandListImpl::UpdateSubresourceData(const std::shared_ptr<Texture>& texture, const TextureSubresourceData& subresourceData)
            {
                ASSERT(texture);
                ASSERT(subresourceData.subresourceIndex < texture->GetDescription().GetNumSubresources());
                ASSERT(subresourceData.allocation);

                const auto resourceImpl = texture->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourceImpl);

                size_t intermediateDataOffset = 0;
                switch (subresourceData.allocation->GetMemoryType())
                {
                case MemoryAllocation::Type::UploadBuffer:
                {
                    const auto allocation = subresourceData.allocation->GetPrivateImpl<GpuMemoryHeap::Allocation>();
                    intermediateDataOffset = allocation->offset;
                    break;
                }
                default:
                    LOG_FATAL("Unsupported memory type");
                }

                const auto& device = DeviceContext::GetDevice();
                auto desc = resourceImpl->GetD3DObject()->GetDesc();
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
                UINT numRows;
                device->GetCopyableFootprints(&desc, subresourceData.subresourceIndex, 1, intermediateDataOffset, &layout, &numRows, nullptr, nullptr);

                ASSERT(subresourceData.depthPitch == layout.Footprint.RowPitch * numRows);
                ASSERT(subresourceData.rowPitch == layout.Footprint.RowPitch);

                switch (subresourceData.allocation->GetMemoryType())
                {
                case MemoryAllocation::Type::UploadBuffer:
                {
                    const auto allocation = subresourceData.allocation->GetPrivateImpl<GpuMemoryHeap::Allocation>();
                    ASSERT(allocation->offset == layout.Offset);

                    CD3DX12_TEXTURE_COPY_LOCATION dst(resourceImpl->GetD3DObject().get(), subresourceData.subresourceIndex);
                    CD3DX12_TEXTURE_COPY_LOCATION src(allocation->resource.get(), layout);

                    D3DCommandList_.get()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
                    break;
                }
                default:
                    LOG_FATAL("Unsupported memory type");
                    break;
                }
                // UpdateSubresources<1>(D3DCommandList_.get(), resourceImpl->GetD3DObject().get(), alloc.resource.get(), alloc.offset, subresourceIndex, 1, &subresourceData);
                /*
                
                    if (true) // is cpumemory
                    {
                        const auto intermediateSize = GetRequiredIntermediateSize(resourceImpl->GetD3DObject().get(), subresourceIndex, 1);

                        const auto& alloc = DeviceContext::GetUploadHeap()->Allocate(intermediateSize);

                        D3D12_SUBRESOURCE_DATA subresourceData;

                        // subresourceData.pData = subresourceData.data;
                        subresourceData.RowPitch = subresourceData.rowPitch;
                        subresourceData.SlicePitch = subresourceData.depthPitch;

                        UpdateSubresources<1>(D3DCommandList_.get(), resourceImpl->GetD3DObject().get(), alloc.resource.get(), alloc.offset, subresourceIndex, 1, &subresourceData);
                    }*/
            }

            void CommandListImpl::ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color)
            {
                ASSERT(renderTargetView);
                ASSERT(D3DCommandList_);

                const auto allocation = renderTargetView->GetPrivateImpl<DescriptorHeap::Allocation>();
                ASSERT(allocation);

                const auto& resource = renderTargetView->GetGpuResource().lock();
                ASSERT(resource);
                ASSERT(resource->GetGpuResourceType() == GpuResource::Type::Texture);

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