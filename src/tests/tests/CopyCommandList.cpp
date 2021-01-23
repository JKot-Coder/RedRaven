#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include <catch2/catch.hpp>

#include "ApprovalTests/ApprovalTests.hpp"

#include "render/RenderContext.hpp"
#include "gapi/CommandList.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Tests

    {
        TEST_CASE_METHOD(TestContextFixture, "CopyCommmanList", "[CommandList][CopyCommmanList]")
        {
        
            Vector2 qwe = Vector2::UNIT_X;
            qwe[0];

            auto& renderContext = Render::RenderContext::Instance();
            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                 REQUIRE(commandList->Close() == GAPI::Result::Ok);                
            }
        }

        TEST_CASE("HelloApprovals")
        {
            //     ApprovalTests::Approvals::verify("Hello Approvals!");
        }
    }
}