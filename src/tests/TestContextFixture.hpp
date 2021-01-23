#pragma once

#include "gapi/ForwardDeclarations.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        class TestContextFixture
        {
        public:
            TestContextFixture() = default;
            ~TestContextFixture() = default;

        protected:
            void submitAndWait(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList);
        };
    }
}