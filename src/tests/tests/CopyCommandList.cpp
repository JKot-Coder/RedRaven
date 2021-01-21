#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include <catch2/catch.hpp>

#include "ApprovalTests/ApprovalTests.hpp"

#include "render/RenderContext.hpp"
#include "gapi/CommandList.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        TEST_CASE_METHOD(TestContextFixture, "CopyCommmanList tests", "[CommandList][CopyCommmanList]")
        {
            auto& renderContext = Render::RenderContext::Instance();
            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();                
            }
        }

        TEST_CASE("HelloApprovals")
        {
            //     ApprovalTests::Approvals::verify("Hello Approvals!");
        }
    }
}