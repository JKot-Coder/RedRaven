#pragma once

#include "common/threading/Mutex.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class CommandListImpl;

            class InitialDataUploder final
            {
            public:
                void Init();
                void DefferedUploadTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<IntermediateMemory>& textureData);

                void PerformUploads();

            private:
                std::unique_ptr<CommandListImpl> commandList_;
                std::atomic_bool isWaitingDefferdCommands;
                Threading::Mutex mutex_;
            };
        }
    }
}