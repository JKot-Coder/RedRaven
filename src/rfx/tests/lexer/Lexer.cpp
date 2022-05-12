#include "TestUtils.hpp"
#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("LexerTests", "[Lexer]")
        {
            runTestOnFile("../src/rfx/tests/lexer/test.rfx", "../src/rfx/tests/lexer", TestType::LexerTest);
            //  runTestsInDirectory("../src/rfx/tests/lexer");
        }
    }
}