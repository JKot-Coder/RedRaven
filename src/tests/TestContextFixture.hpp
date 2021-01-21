#pragma once

namespace OpenDemo::Common::Debug
{
    struct MemorySnapshot;
}

namespace Tests
{
    class TestContextFixture
    {
    public:
        TestContextFixture();
        ~TestContextFixture();

        bool Init();
    };
}