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
                  DL::IRenderDevice* device,
                  DL::IEngineFactory* engineFactory,
                  DL::IDeviceContext* immediateContext,
                  const GAPI::SwapChainDesc& desc,
                  uint32_t frameLatency);

        void InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const override;
        void InitDepthBufferTexture(Texture& resource) const override;
        void Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFER_COUNT>& backBuffers, GAPI::Texture* depthBuffer) override;

        virtual eastl::any GetWaitableObject() const override;
        uint32_t GetCurrentBackBufferIndex() const override;

        void Present();

    private:
        void resetRTVs();

    private:
        uint32_t backBufferCount = 0;
        eastl::fixed_vector<DL::ITextureView*, 4> rtvs;
        DL::RefCntAutoPtr<DL::ISwapChain> swapChain;
    };
}
