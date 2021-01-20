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

        void begin();
        void end();

    private:
        std::shared_ptr<MemorySnapshot> startSnapshot;
        std::shared_ptr<MemorySnapshot> finishSnapshot;
    };
}