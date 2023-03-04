#pragma once

namespace RR::Rfx
{
    class NodeBase
    {
    };

    struct Scope : public NodeBase
    {
        Scope* parent = nullptr;
    };
}