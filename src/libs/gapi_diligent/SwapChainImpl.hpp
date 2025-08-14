#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/SwapChain.hpp"

#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
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
                  const SwapChainDescription& description, const std::string& name);

        void Reset(const SwapChainDescription& description, const std::array<eastl::shared_ptr<Texture>, MAX_BACK_BUFFER_COUNT>& backBuffers) override;

        virtual eastl::any GetWaitableObject() const override;
        uint32_t GetCurrentBackBufferIndex() const override;

        void InitBackBufferTexture(uint32_t backBufferIndex, const eastl::shared_ptr<Texture>& resource) override;

        void Present();

    private:
        DL::RefCntAutoPtr<DL::ISwapChain> swapChain;
    };
}
