#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/commands/Command.hpp"

#include "gapi/GpuResourceViews.hpp"
#include "math/VectorMath.hpp"

namespace RR::GAPI::Commands
{
    struct ClearRTV : public Command
    {
        ClearRTV(const RenderTargetView* rtv, const Vector4& color)
            : Command(Command::Type::ClearRenderTargetView),
              color(color)
        {
            ASSERT(rtv);
            rtvImpl = rtv->GetPrivateImpl<IGpuResourceView>();
        };

        const IGpuResourceView* rtvImpl;
        Vector4 color;
    };
    static_assert(std::is_trivially_copyable<ClearRTV>::value);
}