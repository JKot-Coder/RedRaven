#include "CommandListImpl.hpp"

#include "d3dx12.h"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"

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

                D3DCallMsg(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())), "CreateCommandAllocator");

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

                D3DCallMsg(DeviceContext::GetDevice()->CreateCommandList(0, type_, allocator.get(), nullptr, IID_PPV_ARGS(D3DCommandList_.put())), "CreateCommandList");

                D3DUtils::SetAPIName(D3DCommandList_.get(), name);
            }

            void CommandListImpl::ResetAfterSubmit(CommandQueueImpl& commandQueue)
            {
                ASSERT(D3DCommandList_);

                commandAllocatorsPool_.ResetAfterSubmit(commandQueue);
                const auto& allocator = commandAllocatorsPool_.GetNextAllocator();
                D3DCallMsg(D3DCommandList_->Reset(allocator.get(), nullptr), "Reset");
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

            void CommandListImpl::UpdateTextureData(const std::shared_ptr<Texture>& texture, const std::vector<TextureSubresourceFootprint>& subresourceFootprint)
            {
                ASSERT(texture);
                ASSERT(texture->GetDescription().GetNumSubresources() == subresourceFootprint.size());

                UpdateSubresourceData(texture, 0, subresourceFootprint);
            }

            void CommandListImpl::UpdateSubresourceData(const std::shared_ptr<Texture>& texture, uint32_t firstSubresource, const std::vector<TextureSubresourceFootprint>& subresourceFootprint)
            {
                ASSERT(texture);
                ASSERT(firstSubresource + subresourceFootprint.size() <= texture->GetDescription().GetNumSubresources());

                const auto subresourcesCount = subresourceFootprint.size();
                ASSERT(subresourcesCount > 0);

                const auto resourceImpl = texture->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourceImpl);

                if (true)
                {
                    const auto intermediateSize = GetRequiredIntermediateSize(resourceImpl->GetD3DObject().get(), firstSubresource, subresourceFootprint.size());

                   // DeviceContext::GetUploadHeap()->Allocate(intermediateSize);

                        std::vector<D3D12_SUBRESOURCE_DATA>
                            subresourcesData(subresourcesCount);

                    for (int index = 0; index < subresourcesCount; index++)
                    {
                        auto& subresourceData = subresourcesData[index];

                        subresourceData.pData = subresourceFootprint[index].data;
                        subresourceData.RowPitch = subresourceFootprint[index].rowPitch;
                        subresourceData.SlicePitch = subresourceFootprint[index].depthPitch;
                    }
                }

              //  deviceContext.getUploadBuffer();
               // UpdateSubresources();
                //    UpdateSubresources(D3DCommandList_, resourceImpl->GetD3DObject().get(), buffer, intermediateOffset, firstSubresource, subresourcesCount, &subresourcesData[0]);
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
                D3DCallMsg(D3DCommandList_->Close(), "Close");
            }
        };
    }
}