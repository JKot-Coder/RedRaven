#pragma once

#include "common/Math.hpp"
#include "gapi/CommandContext.hpp"
#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            class CommandContextImpl final : public CommandContextInterface
            {
            public:
                CommandContextImpl();

                Result Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name);

                void Reset() override;
                void Close() override;

                void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

                const ComSharedPtr<ID3D12GraphicsCommandList>& getD3DCommandList() const { return D3DCommandList_; }

            private:
                std::unique_ptr<CommandListImpl> commandList_;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_ = nullptr;
            };

        };
    }
}
