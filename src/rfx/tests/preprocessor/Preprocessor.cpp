#include "TestUtils.hpp"
#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("PreprocessorTests", "[Preprocessor]")
        {
            runTestsInDirectory("../src/rfx/tests/preprocessor", TestType::CommandLine);
        }
    }
}