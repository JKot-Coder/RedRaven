#pragma once


#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandList.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    struct IDeviceContext;
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
        void Compile(GAPI::CommandList& commandList,DL::IDeviceContext* device);

        DL::ICommandList* GetCommandList() { ASSERT(diligentCommandList); return diligentCommandList.RawPtr(); }
        void Reset() { diligentCommandList.Release(); }

    private:
        DL::RefCntAutoPtr<DL::ICommandList> diligentCommandList;
    };
}
