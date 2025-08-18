#pragma once

#include "gapi/GpuResourceViews.hpp"

namespace Diligent
{
    class ITextureView;
    class IBufferView;
}
namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    class GpuResourceViewImpl final : public IGpuResourceView
    {
    public:
        GpuResourceViewImpl() = default;
        ~GpuResourceViewImpl();

        void Init(GAPI::GpuResourceView& resource);

        DL::ITextureView* GetTextureView() const { return textureView; }
        DL::IBufferView* GetBufferView() const { return bufferView; }

    private:
        union
        {
            DL::ITextureView* textureView;
            DL::IBufferView* bufferView;
        };
    };
}