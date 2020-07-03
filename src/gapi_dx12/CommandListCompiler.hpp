#pragma once

#include "gapi/Device.hpp"

struct ID3D12Device;
struct ID3D12CommandList;

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                class CommandListImpl;

                struct CommandListCompilerContext
                {
                    CommandListCompilerContext() = delete;
                    CommandListCompilerContext(ID3D12Device* device, CommandList* commandList);

                private:
                    ID3D12Device* device_;
                    CommandList* commandList_;
                    CommandListImpl* commandListImpl_;
                    ID3D12CommandList* d3dCommandList_;
                };

                namespace CommandListCompiler
                {
                    GAPIStatus Compile(const CommandListCompilerContext& context);
                }

            }
        }
    }
}