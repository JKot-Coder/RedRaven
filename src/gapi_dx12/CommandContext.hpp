#pragma once

#include "common/Math.hpp"

#include "gapi/RenderCommandContextInterface.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class RenderCommandContext final : public RenderCommandContextInterface
            {
            public:
                RenderCommandContext();

                Result Init(ID3D12Device* device, const U8String& name);

                void Reset() override;

                void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) override;

            private:
                std::unique_ptr<CommandListImpl> commandList_;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_ = nullptr;
            };
        };
    }
}
