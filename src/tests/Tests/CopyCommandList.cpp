#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include "ApprovalIntegration/ImageApprover.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"
#include "common/OnScopeExit.hpp"

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
            Vector4 checkerboardPattern(Vector3u texel, uint32_t subresource)
            {
                if (((texel.x / 4) + (texel.y / 4) + (texel.z / 4) + subresource) & 1)
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

                return colors[subresource % colors.size()];
            }

            template <>
            uint32_t checkerboardPattern(Vector3u texel, uint32_t level)
            {
                const auto& value = checkerboardPattern<Vector4>(texel, level);
                // TODO olor class
                // RGBA format
                return static_cast<uint32_t>(value.w * 255.0f) << 24 |
                       static_cast<uint32_t>(value.z * 255.0f) << 16 |
                       static_cast<uint32_t>(value.y * 255.0f) << 8 |
                       static_cast<uint32_t>(value.x * 255.0f);
            }

            template <typename T>
            void fillTextureData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& textureData)
            {
                ASSERT(textureData->GetFirstSubresource() == 0);
                ASSERT((std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA8Uint) ||
                       (std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::BGRA8Unorm) ||
                       (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA16Float) ||
                       (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA32Float));

                const auto& subresourceFootprints = textureData->GetSubresourceFootprints();
                const auto dataPointer = static_cast<uint8_t*>(textureData->GetAllocation()->Map());

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];

                    for (uint32_t depth = 0; depth < subresourceFootprint.depth; depth++)
                    {
                        const auto depthPointer = dataPointer + subresourceFootprint.offset +
                                                  depth * subresourceFootprint.depthPitch;
                        for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                        {
                            const auto rowPointer = depthPointer + row * subresourceFootprint.rowPitch;
                            auto columnPointer = reinterpret_cast<T*>(rowPointer);

                            for (uint32_t column = 0; column < subresourceFootprint.width; column++)
                            {
                                const auto texel = Vector3u(column, row, depth);

                                *columnPointer = checkerboardPattern<T>(texel, index);
                                columnPointer++;
                            }
                        }
                    }
                }

                textureData->GetAllocation()->Unmap();
            }

            void initTextureData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& textureData)
            {
                switch (description.GetFormat())
                {
                case GAPI::GpuResourceFormat::RGBA8Uint:
                case GAPI::GpuResourceFormat::BGRA8Unorm:
                    fillTextureData<uint32_t>(description, textureData);
                    break;
                case GAPI::GpuResourceFormat::RGBA16Float:
                case GAPI::GpuResourceFormat::RGBA32Float:
                    fillTextureData<Vector4>(description, textureData);
                    break;
                default:
                    LOG_FATAL("Unsupported format");
                }
            }

            bool isSubresourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs, uint32_t lSubresourceIndex,
                                    const GAPI::CpuResourceData::SharedPtr& rhs, uint32_t rSubresourceIndex)
            {
                ASSERT(lhs);
                ASSERT(rhs);
                ASSERT(lhs != rhs);
                ASSERT(lSubresourceIndex < lhs->GetNumSubresources());
                ASSERT(rSubresourceIndex < lhs->GetNumSubresources());
                ASSERT(lhs->GetAllocation()->GetMemoryType() != GAPI::MemoryAllocationType::Upload);
                ASSERT(rhs->GetAllocation()->GetMemoryType() != GAPI::MemoryAllocationType::Upload);

                const auto ldataPointer = static_cast<uint8_t*>(lhs->GetAllocation()->Map());
                const auto rdataPointer = static_cast<uint8_t*>(rhs->GetAllocation()->Map());

                ON_SCOPE_EXIT(
                    {
                        lhs->GetAllocation()->Unmap();
                        rhs->GetAllocation()->Unmap();
                    });

                const auto& lfootprint = lhs->GetSubresourceFootprintAt(lSubresourceIndex);
                const auto& rfootprint = rhs->GetSubresourceFootprintAt(rSubresourceIndex);

                ASSERT(lfootprint.isComplatable(rfootprint));

                auto lrowPointer = ldataPointer + lfootprint.offset;
                auto rrowPointer = rdataPointer + rfootprint.offset;

                for (uint32_t row = 0; row < lfootprint.numRows; row++)
                {
                    if (memcmp(lrowPointer, rrowPointer, lfootprint.rowSizeInBytes) != 0)
                        return false;

                    lrowPointer += lfootprint.rowPitch;
                    rrowPointer += rfootprint.rowPitch;
                }

                return true;
            }

            bool isResourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs,
                                 const GAPI::CpuResourceData::SharedPtr& rhs)
            {
                ASSERT(lhs != rhs);
                ASSERT(lhs->GetNumSubresources() == rhs->GetNumSubresources());

                const auto numSubresources = lhs->GetNumSubresources();
                for (uint32_t index = 0; index < numSubresources; index++)
                    if (!isSubresourceEqual(lhs, index, rhs, index))
                        return false;

                return true;
            }

            GAPI::GpuResourceDescription createDescription(GAPI::GpuResourceDimension dimension, uint32_t size, GAPI::GpuResourceFormat format)
            {
                const auto numArraySlices = 3;
                switch (dimension)
                {
                case GAPI::GpuResourceDimension::Texture1D:
                    return GAPI::GpuResourceDescription::Create1D(size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2D:
                    return GAPI::GpuResourceDescription::Create2D(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2DMS:
                    return GAPI::GpuResourceDescription::Create2DMS(size, size, format, 2, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::RenderTarget, numArraySlices);
                case GAPI::GpuResourceDimension::Texture3D:
                    return GAPI::GpuResourceDescription::Create3D(size, size, size, format);
                case GAPI::GpuResourceDimension::TextureCube:
                    return GAPI::GpuResourceDescription::CreateCube(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                }

                ASSERT_MSG(false, "Unsupported GpuResourceDimension");
                return GAPI::GpuResourceDescription::Create1D(0, GAPI::GpuResourceFormat::Unknown, GAPI::GpuResourceBindFlags::ShaderResource);
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyCommmandList", "[CommandList][CopyCommmandList]")
        {
            auto& renderContext = Render::RenderContext::Instance();

            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyTextureTests", "[CommandList][CopyCommmandList][CopyTexture]")
        {
            auto& renderContext = Render::RenderContext::Instance();

            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto copyQueue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
            REQUIRE(copyQueue != nullptr);

            std::array<GAPI::GpuResourceFormat, 2> formatsToTest = { GAPI::GpuResourceFormat::RGBA8Uint, GAPI::GpuResourceFormat::RGBA32Float };
            for (const auto format : formatsToTest)
            {
                const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

                const std::array<GAPI::GpuResourceDimension, 4> dimensions = {
                    GAPI::GpuResourceDimension::Texture1D,
                    GAPI::GpuResourceDimension::Texture2D,
                    GAPI::GpuResourceDimension::Texture3D,
                    GAPI::GpuResourceDimension::TextureCube
                };

                const std::array<U8String, 4> dimensionTitles = {
                    "Texture1D",
                    "Texture2D",
                    "Texture3D",
                    "TextureCube"
                };

                for (int idx = 0; idx < dimensions.size(); idx++)
                {
                    const auto dimension = dimensions[idx];
                    const auto dimensionTitle = dimensionTitles[idx];

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Copy texure data on CPU", dimensionTitle, formatName))
                    {
                        const auto& description = createDescription(dimension, 128, format);

                        const auto sourceData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto destData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);

                        initTextureData(description, sourceData);
                        destData->CopyDataFrom(sourceData);

                        REQUIRE(isResourceEqual(sourceData, destData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Upload texure indirect", dimensionTitle, formatName))
                    {
                        const auto& description = createDescription(dimension, 128, format);

                        const auto cpuData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto readbackData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Readback);

                        initTextureData(description, cpuData);

                        auto testTexture = renderContext.CreateTexture(description, GAPI::GpuResourceCpuAccess::None, "Test");

                        commandList->UpdateTexture(testTexture, cpuData);
                        commandList->ReadbackTexture(testTexture, readbackData);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(cpuData, readbackData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Upload texure direct", dimensionTitle, formatName))
                    {
                        const auto& description = createDescription(dimension, 128, format);

                        const auto cpuData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto sourceData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Upload);
                        const auto readbackData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Readback);

                        initTextureData(description, cpuData);
                        sourceData->CopyDataFrom(cpuData);

                        auto testTexture = renderContext.CreateTexture(description, GAPI::GpuResourceCpuAccess::None, "Test");

                        commandList->UpdateTexture(testTexture, sourceData);
                        commandList->ReadbackTexture(testTexture, readbackData);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(cpuData, readbackData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Copy texure on GPU", dimensionTitle, formatName))
                    {
                        const auto& description = createDescription(dimension, 128, format);

                        const auto sourceData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto readbackData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Readback);

                        initTextureData(description, sourceData);

                        auto source = renderContext.CreateTexture(description, GAPI::GpuResourceCpuAccess::None, "Source");
                        auto dest = renderContext.CreateTexture(description, GAPI::GpuResourceCpuAccess::None, "Dest");

                        commandList->UpdateTexture(source, sourceData);
                        commandList->CopyTexture(source, dest);
                        commandList->ReadbackTexture(dest, readbackData);

                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(sourceData, readbackData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] CopyTextureSubresource", dimensionTitle, formatName))
                    {
                        const auto& sourceDescription = createDescription(dimension, 256, format);
                        const auto sourceData = renderContext.AllocateIntermediateTextureData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        auto source = renderContext.CreateTexture(sourceDescription, GAPI::GpuResourceCpuAccess::None, "Source");

                        initTextureData(sourceDescription, sourceData);
                        commandList->UpdateTexture(source, sourceData);

                        const auto& destDescription = createDescription(dimension, 128, format);
                        const auto destData = renderContext.AllocateIntermediateTextureData(destDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        auto dest = renderContext.CreateTexture(destDescription, GAPI::GpuResourceCpuAccess::None, "Dest");

                        initTextureData(destDescription, destData);
                        commandList->UpdateTexture(dest, destData);

                        for (uint32_t index = 0; index < destDescription.GetNumSubresources(); index++)
                        {
                            const auto mipLevel = destDescription.GetSubresourceMipLevel(index);
                            const auto arraySlice = destDescription.GetSubresourceArraySlice(index);
                            const auto face = destDescription.GetSubresourceFace(index);

                            if (mipLevel % 2 != 0)
                                commandList->CopyTextureSubresource(source, sourceDescription.GetSubresourceIndex(arraySlice, mipLevel + 1, face), dest, index);
                        }

                        const auto readbackData = renderContext.AllocateIntermediateTextureData(destDescription, GAPI::MemoryAllocationType::Readback);
                        commandList->ReadbackTexture(dest, readbackData);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);

                        for (uint32_t index = 0; index < destDescription.GetNumSubresources(); index++)
                        {
                            const auto mipLevel = destDescription.GetSubresourceMipLevel(index);
                            const auto arraySlice = destDescription.GetSubresourceArraySlice(index);
                            const auto face = destDescription.GetSubresourceFace(index);

                            bool equal = (mipLevel % 2 != 0) ? isSubresourceEqual(sourceData, sourceDescription.GetSubresourceIndex(arraySlice, mipLevel + 1, face), readbackData, index)
                                                             : isSubresourceEqual(destData, index, readbackData, index);
                            REQUIRE(equal);
                        }
                    }
                }

                DYNAMIC_SECTION(fmt::format("[Texture3D::{}] CopyTextureSubresource", formatName))
                {
                    const auto& sourceDescription = createDescription(GAPI::GpuResourceDimension::Texture3D, 128, format);
                    const auto sourceData = renderContext.AllocateIntermediateTextureData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                    auto source = renderContext.CreateTexture(sourceDescription, GAPI::GpuResourceCpuAccess::None, "Source");

                    initTextureData(sourceDescription, sourceData);
                    commandList->UpdateTexture(source, sourceData);

                    const auto& destDescription = createDescription(GAPI::GpuResourceDimension::Texture3D, 64, format);
                    const auto destData = renderContext.AllocateIntermediateTextureData(destDescription, GAPI::MemoryAllocationType::Upload);
                    auto dest = renderContext.CreateTexture(destDescription, GAPI::GpuResourceCpuAccess::None, "Dest");

                    initTextureData(destDescription, destData);
                    commandList->UpdateTexture(dest, destData);

                    commandList->CopyTextureSubresourceRegion(source, 1, Box3u(7, 8, 13, 32, 32, 32), dest, 0, Vector3u(16, 16, 16));
                    commandList->CopyTextureSubresourceRegion(source, 2, Box3u(0, 0, 0, 16, 16, 16), dest, 1, Vector3u(8, 8, 8));
                    commandList->CopyTextureSubresourceRegion(source, 0, Box3u(3, 42, 66, 8, 8, 8), dest, 2, Vector3u(0, 0, 0));

                    const auto readbackData = renderContext.AllocateIntermediateTextureData(destDescription, GAPI::MemoryAllocationType::Readback);
                    commandList->ReadbackTexture(dest, readbackData);
                    commandList->Close();

                    submitAndWait(copyQueue, commandList);
                    ImageApprover::verify(readbackData);
                }
            }
        }
    }
}