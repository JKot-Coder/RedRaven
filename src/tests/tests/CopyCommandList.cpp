#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include <catch2/catch.hpp>

#include "ApprovalTests/ApprovalTests.hpp"

namespace Tests
{
    TEST_CASE_METHOD(TestContextFixture, "CopyCommmanList tests", "[CommandList][CopyCommmanList]")
    {
        REQUIRE(1 == 1);
        REQUIRE(2 == 2);
        begin();
        REQUIRE(3 == 3);
        REQUIRE(4 == 4);
        REQUIRE(5 == 5);
        REQUIRE(2 == 2);
        REQUIRE(2 == 2);
        REQUIRE(2 == 2);
        REQUIRE(2 == 2);
        REQUIRE(2 == 2);
        REQUIRE(2 == 2);
        end();
       
    }

    TEST_CASE("HelloApprovals")
    {
   //     ApprovalTests::Approvals::verify("Hello Approvals!");
    }
}