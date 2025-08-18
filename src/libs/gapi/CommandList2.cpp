#include "CommandList2.hpp"

#include "gapi/commands/ClearRTV.hpp"

namespace RR::GAPI
{
    void GraphicsOperationsMixin::ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color)
    {
        GetCommandList().emplaceCommand<Commands::ClearRTV>(renderTargetView, color);
    }
}