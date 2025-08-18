#pragma once

namespace RR::GAPI
{
    struct Command
    {
        enum class Type
        {
            ClearRenderTargetView
        };

    public:
        Command(Type type) : type(type) { }

        Type type;
    };
}