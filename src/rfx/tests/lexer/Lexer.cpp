#include "TestUtils.hpp"

#include "common/Result.hpp"

#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("LexerTests", "[Lexer]")
        {
            runTestsInDirectory("../src/rfx/tests/lexer");
        }
    }
}