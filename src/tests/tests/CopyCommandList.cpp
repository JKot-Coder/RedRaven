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

            GAPI::Texture::SharedPtr createTestTexture(const GAPI::TextureDescription& description, const U8String& name, GAPI::GpuResourceCpuAccess cpuAcess = GAPI::GpuResourceCpuAccess::None, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::None)
            {
                auto& renderContext = Render::RenderContext::Instance();

                auto texture = renderContext.CreateTexture(description, bindFlags, {}, cpuAcess, name);
                REQUIRE(texture);

                return texture;
            }

            void initTextureData(const GAPI::IntermediateMemory::SharedPtr& data)
            {
                for (const auto& subresourceFootprint : data->GetSubresourceFootprints())
                {
                    auto rowPointer = static_cast<uint8_t*>(subresourceFootprint.data);
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                        memset(rowPointer, 0, subresourceFootprint.rowPitch);
                        *static_cast<uint32_t*>(subresourceFootprint.data) = 0xDEADBEEF;
                        rowPointer += subresourceFootprint.rowPitch;
                    }
                }
            }

            bool isResourceDataEqual(const GAPI::IntermediateMemory::SharedPtr& lhs,
                                     const GAPI::IntermediateMemory::SharedPtr& rhs)
            {
                ASSERT(lhs->GetNumSubresources() == rhs->GetNumSubresources());

                const auto numSubresources = lhs->GetNumSubresources();
                for (uint32_t index = 0; index < numSubresources; index++)
                {
                    const auto& lfootprint = lhs->GetSubresourceFootprintAt(index);
                    const auto& rfootprint = rhs->GetSubresourceFootprintAt(index);

                    ASSERT(lfootprint.rowSizeInBytes == rfootprint.rowSizeInBytes);
                    ASSERT(lfootprint.rowPitch == rfootprint.rowPitch);
                    ASSERT(lfootprint.depthPitch == rfootprint.depthPitch);
                    ASSERT(lfootprint.numRows == rfootprint.numRows);

                    auto lrowPointer = static_cast<uint8_t*>(lfootprint.data);
                    auto rrowPointer = static_cast<uint8_t*>(rfootprint.data);

                    for (uint32_t row = 0; row < lfootprint.numRows; row++)
                    {
                        if (memcmp(lrowPointer, rrowPointer, lfootprint.rowSizeInBytes) != 0)
                            return false;

                        lrowPointer += lfootprint.rowPitch;
                        rrowPointer += rfootprint.rowPitch;
                    }
                }

                return true;
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

                auto source = createTestTexture(description, "Source");
                auto dest = createTestTexture(description, "Dest");

                commandList->CopyTexture(source, dest);
                commandList->Close();

                submitAndWait(copyQueue, commandList);
            }

            SECTION("CopyTexture_Float")
            {
                const auto& description = GAPI::TextureDescription::Create2D(128, 128, GAPI::GpuResourceFormat::RGBA16Float);

                auto source = createTestTexture(description, "Source");
                auto dest = createTestTexture(description, "Dest");

                const auto sourceData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Upload);
                const auto readbackData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Readback);

                initTextureData(sourceData);

                commandList->UpdateTexture(source, sourceData);
                commandList->CopyTexture(source, dest);
                commandList->ReadbackTexture(dest, readbackData);

                commandList->Close();

                submitAndWait(copyQueue, commandList);
                REQUIRE(isResourceDataEqual(sourceData, readbackData));
            }
        }

        TEST_CASE("HelloApprovals")
        {
            //     ApprovalTests::Approvals::verify("Hello Approvals!");
        }
    }
}