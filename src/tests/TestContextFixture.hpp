#pragma once

namespace OpenDemo
{
    namespace Windowing
    {
        class InputtingWindow;
    }
    
    namespace Tests
    {
        class TestContextFixture
        {
        public:
            TestContextFixture();
            ~TestContextFixture();

            bool Init();

        private:
            std::shared_ptr<Windowing::InputtingWindow> window_;
        };
    }
}