#pragma once

namespace OpenDemo
{
    namespace Windowing
    {
        class InputtingWindow;
    }

    namespace Tests
    {
        class Application
        {
        public:
            int Run(int argc, char** argv);

        private:
            bool init();
            void terminate();

        private:
            std::shared_ptr<Windowing::InputtingWindow> window_;
        };
    }
}