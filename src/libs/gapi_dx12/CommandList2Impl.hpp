#pragma once
#include "gapi/CommandList2.hpp"

namespace RR::GAPI::DX12
{
    class CommandList2Impl final : public ICommandList
    {
    public:
        void Init(const CommandList2& commandList);
    };
}