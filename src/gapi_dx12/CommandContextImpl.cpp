#include "CommandContextImpl.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            CommandContextImpl::CommandContextImpl() : commandList_(new CommandListImpl(D3D12_COMMAND_LIST_TYPE_DIRECT))
            {
            }

            Result CommandContextImpl::Init(const ComSharedPtr<ID3D12Device>& device, const CommandListType commandListType, const U8String& name)
            {
                // TODO
                std::ignore = commandListType;
                D3DCall(commandList_->Init(device, name));

                D3DCommandList_ = commandList_->GetD3DObject();

                return Result::Ok;
            }

            void CommandContextImpl::Reset()
            {
                //  Log::Print::Info("Reset\n");
                ASSERT(D3DCommandList_);
                commandList_->Reset();
                //     commandList_->Submit();
            }

            void CommandContextImpl::ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color)
            {
                ASSERT(renderTargetView);
                ASSERT(D3DCommandList_);

                ASSERT(dynamic_cast<DescriptorHeap::Allocation*>(renderTargetView->GetPrivateImpl()));
                const auto& allocation = static_cast<DescriptorHeap::Allocation*>(renderTargetView->GetPrivateImpl());

                const auto& resource = renderTargetView->GetGpuResource().lock();
                ASSERT(resource);
                ASSERT(resource->GetGpuResourceType() == GpuResource::Type::Texture);

                ASSERT(dynamic_cast<ResourceImpl*>(resource->GetPrivateImpl()));
                const auto resourceImpl = static_cast<ResourceImpl*>(resource->GetPrivateImpl());

                D3D12_RESOURCE_BARRIER barrier
                    = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

                D3DCommandList_->ResourceBarrier(1, &barrier);

                D3DCommandList_->ClearRenderTargetView(allocation->GetCPUHandle(), &color.x, 0, nullptr);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandContextImpl::Close()
            {
                //Log::Print::Info("Close\n");
                // RESULT;
                const auto result = D3DCommandList_->Close();
                std::ignore = result;
            }
        };
    }
}