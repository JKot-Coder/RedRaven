#pragma once

namespace RR::GAPI
{
    struct Command
    {
        enum class Type
        {
            ClearRenderTargetView,
            ClearDepthStencilView,
        };

    public:
        Command(Type type) : type(type) { }

        Type type;
    };
}