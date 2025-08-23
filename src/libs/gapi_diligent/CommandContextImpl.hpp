#pragma once


#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList2.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    class IDeviceContext;
    class ICommandList;
}
namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    class CommandListImpl final : public ICommandList
    {
    public:
        CommandListImpl() = default;
        ~CommandListImpl();

        void Init();
        void Compile(GAPI::CommandList2& commandList,DL::IDeviceContext* device);

        DL::ICommandList* GetCommandList() { ASSERT(commandList); return commandList.RawPtr(); }
        void Reset() { commandList.Release(); }

    private:
        DL::RefCntAutoPtr<DL::ICommandList> commandList;
    };
}
