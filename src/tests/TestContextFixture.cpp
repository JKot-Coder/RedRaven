#include "TestContextFixture.hpp"

#include "common/debug/LeakDetector.hpp"

namespace Tests
{
    TestContextFixture::TestContextFixture()
    {

    }

    TestContextFixture::~TestContextFixture()
    {

    }

    void TestContextFixture::begin()
    {
        const auto& leakDetector = OpenDemo::Common::Debug::LeakDetector::Instance();

        startSnapshot = leakDetector.CreateEmpySnapshot();
        finishSnapshot = leakDetector.CreateEmpySnapshot();

        leakDetector.Capture(startSnapshot);
    }

    void TestContextFixture::end()
    {
        const auto& leakDetector = OpenDemo::Common::Debug::LeakDetector::Instance();
        leakDetector.Capture(finishSnapshot);

        if (leakDetector.GetDifference(startSnapshot, finishSnapshot))
        {
            leakDetector.DumpAllSince(startSnapshot);
        }
    }

}