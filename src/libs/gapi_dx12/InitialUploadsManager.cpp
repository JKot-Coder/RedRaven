#include "InitialUploadsManager.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void InitialUploadsManager::Init()
            {
                Threading::ReadWriteGuard lock(mutex_);

                ASSERT(!commandList_);

                commandList_ = std::make_unique<CommandListImpl>(CommandListType::Graphics);
                commandList_->Init("Initial uploads");
            }

            void InitialUploadsManager::DefferedUploadTexture(const std::shared_ptr<Texture>& texture, const std::shared_ptr<IntermediateMemory>& textureData)
            {
                Threading::ReadWriteGuard lock(mutex_);

                ASSERT(commandList_);
        
                isWaitingDefferdCommands = true;
                commandList_->UpdateTexture(texture, textureData);
            }

            void InitialUploadsManager::PerformUploads()
            {
                ASSERT(commandList_);

                if (!isWaitingDefferdCommands)
                    return;

                Threading::ReadWriteGuard lock(mutex_);
                           
            }

        }
    }
}