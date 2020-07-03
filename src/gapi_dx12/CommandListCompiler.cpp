#include "CommandListCompiler.hpp"

#include "gapi/Command.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/Device.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {

                CommandListCompilerContext::CommandListCompilerContext(ID3D12Device* device, CommandList* commandList)
                    : device_(device)
                    , commandList_(commandList)
                {
                    ASSERT(device)
                    ASSERT(commandList)

                    commandListImpl_ = commandList->GetPrivateImpl<CommandListImpl*>();
                    ASSERT(commandListImpl_);

                    d3dCommandList_ = commandListImpl_->GetCommandList().get();
                    ASSERT(d3dCommandList_);
                }

                template <Command::Type type>
                GAPIStatus CompileCommand(const CommandListCompilerContext& context);

                template <>
                GAPIStatus CompileCommand<Command::Type::CLEAR_RENDER_TARGET>(const CommandListCompilerContext& context)
                {
                    return GAPIStatus::OK;
                }

                template <>
                GAPIStatus CompileCommand<Command::Type::CLEAR_DEPTH_STENCIL>(const CommandListCompilerContext& context)
                {
                    return GAPIStatus::OK;
                }

                GAPIStatus CompileCommand(const Command& command, const CommandListCompilerContext& context)
                {
#define CASE_COMMAND(type)    \
    case Command::Type::type: \
        return CompileCommand<Command::Type::type>(context);
#define CASE_UNIMPLEMENTED_COMMAND(type) \
    case Command::Type::type:            \
        return GAPIStatus::OK;

                    const auto& commandType = command.GetType();

                    switch (commandType)
                    {
                        /*    CASE_COMMAND(DRAW_INDEXED_PRIMITIVE)
                        CASE_COMMAND(DRAW_PRIMITIVE)
                        CASE_COMMAND(CLEAR_RENDER_TARGET)
                        CASE_COMMAND(CLEAR_DEPTH_STENCIL)
                        CASE_COMMAND(CLEAR_UNORDERED_ACCESS)
                        CASE_COMMAND(COPY_BUFFER_REGION)
                        CASE_COMMAND(COPY_TEXTURE)
                        CASE_COMMAND(COPY_TEXTURE_SUBRESOURCE)
                        CASE_COMMAND(RESOLVE_SUBRESOURCE)
                        CASE_COMMAND(UPDATE_BUFFER)
                        CASE_COMMAND(UPDATE_TEXTURE)

                        CASE_COMMAND(QUERY_BEGIN)
                        CASE_COMMAND(QUERY_END)

                        CASE_COMMAND(DISPATCH)

                        CASE_UNIMPLEMENTED_COMMAND(RENDER_CALLBACK)*/

                        CASE_COMMAND(CLEAR_RENDER_TARGET)
                        CASE_COMMAND(CLEAR_DEPTH_STENCIL)

                    default:
                        break;
                    }
#undef CASE_COMMAND
#undef CASE_UNIMPLEMENTED_COMMAND

                    return GAPIStatus::OK;
                }

                GAPIStatus CommandListCompiler::Compile(const CommandListCompilerContext& context)
                {

                    return GAPIStatus::OK;
                }
            }
        }
    }
}