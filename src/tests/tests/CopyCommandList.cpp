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

            template <typename T>
            T checkerboardPattern(Vector3u texel, uint32_t level);

            template <>
            uint32_t checkerboardPattern(Vector3u texel, uint32_t level)
            {
                const auto& value = checkerboardPattern<Vector4>(texel, level);
                // TODO olor class
                // RGBA format
                return static_cast<uint32_t>(value.x * 255.0f) << 24 |
                       static_cast<uint32_t>(value.y * 255.0f) << 16 |
                       static_cast<uint32_t>(value.z * 255.0f) << 8 |
                       static_cast<uint32_t>(value.w * 255.0f);
            }

            template <>
            Vector4 checkerboardPattern(Vector3u texel, uint32_t level)
            {
                if ((texel.x + texel.y + texel.z) / 8 + level & 1 == 0)
                {
                    return Vector4(0.5f, 0.5f, 0.5f, 0.5f);
                }

                std::array<Vector4, 8> colors = {
                    Vector4(0, 0, 1, 1),
                    Vector4(0, 1, 0, 1),
                    Vector4(1, 0, 0, 1),
                    Vector4(0, 1, 1, 1),
                    Vector4(1, 0, 1, 1),
                    Vector4(1, 1, 0, 1),
                    Vector4(1, 1, 1, 1),
                    Vector4(0.25, 0.25, 0.25, 1),
                };

                return colors[std::min(level, 7u)];
            }

            GAPI::Texture::SharedPtr CreateTestTexture(const GAPI::TextureDescription& description, const U8String& name, GAPI::GpuResourceCpuAccess cpuAcess = GAPI::GpuResourceCpuAccess::None, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::None)
            {
                auto& renderContext = Render::RenderContext::Instance();

                auto texture = renderContext.CreateTexture(description, bindFlags, {}, cpuAcess, name);
                REQUIRE(texture);

                return texture;
            }

            void UpdateTexture(const GAPI::Texture::SharedPtr& texture, const GAPI::CopyCommandList::SharedPtr& commandList)
            {
                auto& renderContext = Render::RenderContext::Instance();
                
                const auto& description = texture->GetDescription();
                const auto textureData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Upload);
             
                ASSERT(description.format == GAPI::GpuResourceFormat::RGBA8Uint);

                for (const auto& subresourceFootprint : textureData->GetSubresourceFootprints())
                {
                    auto rowPointer = static_cast<uint8_t*>(subresourceFootprint.data);
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                        auto columnPointer = reinterpret_cast<uint32_t*>(rowPointer);
                        for (uint32_t column = 0; column < description.width; column++)
                        {
                          //  *columnPointer = checkerboardPattern<uint32_t>();
                            columnPointer++;
                        }

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
                // UpdateTexture(dest, commandList);

                commandList->CopyTexture(source, dest);

                ReadBack(dest, commandList, textureData);

                commandList->Close();

                submitAndWait(copyQueue, commandList);
                renderContext.WaitForGpu();
                renderContext.WaitForGpu();
                renderContext.WaitForGpu();

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