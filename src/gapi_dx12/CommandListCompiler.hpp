#pragma once

#include "gapi/DeviceInterface.hpp"

struct ID3D12Device;
struct ID3D12CommandList;

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandListImpl;

            struct CommandListCompilerContext
            {
                CommandListCompilerContext() = delete;
                CommandListCompilerContext(ID3D12Device* device);

            private:
                ID3D12Device* device_;
                CommandListImpl* commandListImpl_;
                ID3D12CommandList* d3dCommandList_;
            };

            namespace CommandListCompiler
            {
                Result Compile(const CommandListCompilerContext& context);
            }
        }
    }
}