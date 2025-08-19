#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"

#include "EASTL/fixed_vector.h"

namespace DL = ::Diligent;

namespace Diligent
{
    class ISwapChain;
}

namespace RR::GAPI::Diligent
{
    class SwapChainImpl final : public ISwapChain
    {
    public:
        SwapChainImpl() = default;
        ~SwapChainImpl();

        void Init(DL::RENDER_DEVICE_TYPE deviceType,
                  const DL::RefCntAutoPtr<DL::IRenderDevice>& device,
                  const DL::RefCntAutoPtr<DL::IEngineFactory>& engineFactory,
                  DL::IDeviceContext* immediateContext,
                  const SwapChainDescription& description,
                  uint32_t frameLatency);

        void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const override;
        void Reset(const SwapChainDescription& description, const Texture** backBuffers) override;

        virtual eastl::any GetWaitableObject() const override;
        uint32_t GetCurrentBackBufferIndex() const override;

        void Present();

    private:
        eastl::fixed_vector<DL::ITextureView*, 4> rtvs;
        DL::RefCntAutoPtr<DL::ISwapChain> swapChain;
    };
}
