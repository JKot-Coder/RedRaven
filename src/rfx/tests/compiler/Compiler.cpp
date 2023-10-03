#include "TestUtils.hpp"
#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("Compiler", "[Compiler]")
        {
            runTestsInDirectory("../src/rfx/tests/compiler");
        }
    }
}