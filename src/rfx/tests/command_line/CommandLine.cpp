#include "TestUtils.hpp"
#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("CommandLine", "[CommandLine]")
        {
            runTestsInDirectory("../src/rfx/tests/command_line");
        }
    }
}