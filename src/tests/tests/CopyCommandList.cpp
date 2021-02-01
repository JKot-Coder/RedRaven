#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include <catch2/catch.hpp>

#include "ApprovalTests/ApprovalTests.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        namespace
        {
            template <typename T>
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

            void UpdateTexture(const GAPI::Texture::SharedPtr& texture, const GAPI::CopyCommandList::SharedPtr& commandList)
            {
                auto& renderContext = Render::RenderContext::Instance();
                const auto textureData = renderContext.AllocateIntermediateTextureData(texture->GetDescription(), GAPI::MemoryAllocationType::Upload);

                for (const auto& subresourceFootprint : textureData->GetSubresourceFootprints())
                {
                    auto rowPointer = static_cast<uint8_t*>(subresourceFootprint.data);
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                        memset(rowPointer, 0, subresourceFootprint.rowPitch);
                        *static_cast<uint32_t*>(subresourceFootprint.data) = 0xDEADBEEF;
                        rowPointer += subresourceFootprint.rowPitch;
                    }
                }

                commandList->UpdateTexture(texture, textureData);
            }

            void ReadBack(const GAPI::Texture::SharedPtr& texture, const GAPI::CopyCommandList::SharedPtr& commandList, const std::shared_ptr<GAPI::IntermediateMemory>& textureData)
            {
                auto& renderContext = Render::RenderContext::Instance();

                commandList->ReadbackTexture(texture, textureData);
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
                commandList->Close();
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

                const auto textureData = renderContext.AllocateIntermediateTextureData(source->GetDescription(), GAPI::MemoryAllocationType::Readback);

                UpdateTexture(source, commandList);
                ReadBack(source, commandList, textureData);

                commandList->CopyTexture(source, dest);

                commandList->Close();

                submitAndWait(copyQueue, commandList);

                for (const auto& subresourceFootprint : textureData->GetSubresourceFootprints())
                {
                    auto rowPointer = static_cast<uint8_t*>(subresourceFootprint.data);
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                    }
                }
            }
        }

        TEST_CASE("HelloApprovals")
        {
            //     ApprovalTests::Approvals::verify("Hello Approvals!");
        }
    }
}