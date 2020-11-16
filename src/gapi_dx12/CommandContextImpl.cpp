#include "CommandContextImpl.hpp"

#include "gapi/Resource.hpp"
#include "gapi/ResourceViews.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            CommandContextImpl::CommandContextImpl() : commandList_(new CommandListImpl(D3D12_COMMAND_LIST_TYPE_DIRECT))
            {
            }

            Result CommandContextImpl::Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name)
            {
                D3DCall(commandList_->Init(device, name));

                D3DCommandList_ = commandList_->GetCommandList();

                return Result::Ok;
            }

            void CommandContextImpl::Reset()
            {
                ASSERT(D3DCommandList_);
                commandList_->Reset();
                //     commandList_->Submit();
            }

            void CommandContextImpl::ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color)
            {
                ASSERT(renderTargetView);
                ASSERT(D3DCommandList_);

                const auto allocation = renderTargetView->GetPrivateImpl<DescriptorHeap::Allocation>();
                ASSERT(allocation);

                const auto& resource = renderTargetView->GetResource().lock();
                ASSERT(resource);
                ASSERT(resource->GetResourceType() == Resource::ResourceType::Texture);

                const auto resourceImpl = resource->GetPrivateImpl<ResourceImpl>();

                D3D12_RESOURCE_BARRIER barrier
                    = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->getD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

                D3DCommandList_->ResourceBarrier(1, &barrier);

                D3DCommandList_->ClearRenderTargetView(allocation->GetCPUHandle(), &color.x, 0, nullptr);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->getD3DObject().get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);
            }

            void CommandContextImpl::Close()
            {
                // RESULT;
                const auto result = D3DCommandList_->Close();
                std::ignore = result;
            }
        };
    }
}