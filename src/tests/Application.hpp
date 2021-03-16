#pragma once

#include "gapi/CommandQueue.hpp"
#include "gapi/ForwardDeclarations.hpp"

#include "common/Singleton.hpp"

namespace OpenDemo
{
    namespace WindowSystem
    {
        class InputtingWindow;
    }

    namespace Tests
    {
        class Application : public Singleton<Application>
        {
        public:
            int Run(int argc, char** argv);

            std::shared_ptr<GAPI::CommandQueue> GetGraphicsCommandQueue(GAPI::CommandQueueType type)
            {
                ASSERT(type != GAPI::CommandQueueType::Count);
                return commandQueue_[static_cast<size_t>(type)];
            }

        private:
            bool init();
            void terminate();

        private:
            std::shared_ptr<WindowSystem::InputtingWindow> window_;
            std::array<std::shared_ptr<GAPI::CommandQueue>, static_cast<size_t>(GAPI::CommandQueueType::Count)> commandQueue_;
        };
    }
}