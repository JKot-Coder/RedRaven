#pragma once

namespace RR::GAPI
{
    struct Command
    {
        enum class Type
        {
            ClearRenderTargetView,
            ClearDepthStencilView,
            Draw
        };

    public:
        Command(Type type) : type(type) { }

        Type type;
    };
}