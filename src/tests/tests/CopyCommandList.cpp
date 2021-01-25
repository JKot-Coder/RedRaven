#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include <catch2/catch.hpp>

#include "ApprovalTests/ApprovalTests.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        namespace
        {
            template<typename T>
            T texelZeroFill(Vector3u texel, uint32_t level)
            {
                return T(0);
            }

            GAPI::Texture::SharedPtr CreateTestTexture(const GAPI::TextureDescription& description, const U8String& name, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::None)
            {
                auto& renderContext = Render::RenderContext::Instance();
  
                auto texture = renderContext.CreateTexture(description, bindFlags, {}, name);
                REQUIRE(texture);

                return texture;
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyCommmanList", "[CommandList][CopyCommmanList]")
        {
            auto& renderContext = Render::RenderContext::Instance();

            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto copyQueue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
            REQUIRE(copyQueue != nullptr);

            SECTION("Close")
            {
                REQUIRE(commandList->Close() == GAPI::Result::Ok);
            }

            SECTION("CopyTexture_RGBA8")
            {
                const auto& description = GAPI::TextureDescription::Create2D(128, 128, GAPI::GpuResourceFormat::RGBA8Uint);

                auto source = CreateTestTexture(description, "Source");
                auto dest = CreateTestTexture(description, "Dest");

                commandList->CopyTexture(source, dest);
                commandList->Close();

                submitAndWait(copyQueue, commandList);
            }

            SECTION("CopyTexture_Float")
            {             
                const auto& description = GAPI::TextureDescription::Create2D(128, 128, GAPI::GpuResourceFormat::RGBA16Float);

                auto source = CreateTestTexture(description, "Source");
                auto dest = CreateTestTexture(description, "Dest");

                commandList->CopyTexture(source, dest);
                commandList->Close();

                submitAndWait(copyQueue, commandList);
            }
        }

        TEST_CASE("HelloApprovals")
        {
            //     ApprovalTests::Approvals::verify("Hello Approvals!");
        }
    }
}