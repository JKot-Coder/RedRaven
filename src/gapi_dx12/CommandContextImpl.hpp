#pragma once

#include "common/Math.hpp"

#include "gapi/CommandContextInterface.hpp"

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

                Result Init(ID3D12Device* device, const U8String& name);

                void Reset() override;
                void Close() override;

                void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) override;

            private:
                std::unique_ptr<CommandListImpl> commandList_;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_ = nullptr;
            };

        };
    }
}
