#include "RenderContext.hpp"

#include "gapi/ResourceViews.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            RenderContext::RenderContext() : commandList_(new CommandListImpl(D3D12_COMMAND_LIST_TYPE_DIRECT))
            {
            }

            GAPIStatus RenderContext::Init(ID3D12Device* device, const U8String& name)
            {
                GAPIStatus result;

                if (GAPIStatusU::Failure(result = GAPIStatus(commandList_->Init(device, name))))
                    return result;

                D3DCommandList_ = commandList_->GetCommandList();

                return result;
            }

            void RenderContext::Reset()
            {
                ASSERT(D3DCommandList_);
                //     commandList_->Submit();
            }

            void RenderContext::ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color)
            {
                ASSERT(D3DCommandList_);

                const auto descriptor = renderTargetView.GetPrivateImpl<DescriptorHeap::Allocation>();
                ASSERT(descriptor);                
             
                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(nullptr, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

                D3DCommandList_->ResourceBarrier(1, &barrier);

                D3DCommandList_->ClearRenderTargetView(descriptor->GetCPUHandle(), &color.x, 0, nullptr);

                barrier = CD3DX12_RESOURCE_BARRIER::Transition(nullptr, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                D3DCommandList_->ResourceBarrier(1, &barrier);

            }
            /*
            void RenderContext::Close()
            {
                commandList_->Close();
            }*/
        };
    }
}