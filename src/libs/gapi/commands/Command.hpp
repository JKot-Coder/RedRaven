#pragma once

namespace RR::GAPI
{
    struct Command
    {
        enum class Type
        {
            Draw,
            DrawIndexed,
            BeginRenderPass,
            EndRenderPass
        };

    public:
        Command(Type type) : type(type) { }

        Type type;
    };
}