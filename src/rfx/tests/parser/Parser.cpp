#include "TestUtils.hpp"
#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("ParserTests", "[Parser]")
        {
            runTestsInDirectory("../src/rfx/tests/parser");
        }
    }
}