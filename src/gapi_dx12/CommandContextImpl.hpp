#pragma once

#include "common/Math.hpp"

#include "gapi/CommandList.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandContextImpl final : public IGraphicsCommandList
            {
            public:
                CommandContextImpl();

                Result Init(const ComSharedPtr<ID3D12Device>& device, const CommandListType commandListType, const U8String& name);

                void Reset() override;
                void Close() override;

                void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

                const ComSharedPtr<ID3D12GraphicsCommandList>& GetD3DObject() const { return D3DCommandList_; }

            private:
                std::unique_ptr<CommandListImpl> commandList_;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_ = nullptr;
            };
        };
    }
}