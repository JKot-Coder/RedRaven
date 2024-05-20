#pragma once

#include "common/Singleton.hpp"

namespace RR
{
    namespace WindowSystem
    {
        class InputtingWindow;
    }

    namespace Tests
    {
        class Application : public Common::Singleton<Application>
        {
        public:
            int Run(int argc, char** argv);

        private:
            bool init();
            void terminate();

        private:
            std::shared_ptr<WindowSystem::InputtingWindow> window_;
        };
    }
}