#include "ComputeCommandList.hpp"

#include "TestContextFixture.hpp"

#include "ApprovalIntegration/ImageApprover.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

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
            void fillColorData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& resourceData)
            {
                ASSERT(resourceData->GetFirstSubresource() == 0);
                ASSERT((std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA8Uint) ||
                       (std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::BGRA8Unorm) ||
                       (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA16Float) ||
                       (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA32Float));

                const auto& subresourceFootprints = resourceData->GetSubresourceFootprints();
                const auto dataPointer = static_cast<uint8_t*>(resourceData->GetAllocation()->Map());
                const auto blockSize = GAPI::GpuResourceFormatInfo::GetBlockSize(description.GetFormat());

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];
                    ASSERT(subresourceFootprint.width * blockSize == subresourceFootprint.rowSizeInBytes);

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

                resourceData->GetAllocation()->Unmap();
            }

            void fillBufferData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& resourceData)
            {
                ASSERT(description.GetDimension() == GAPI::GpuResourceDimension::Buffer);

                const auto& subresourceFootprints = resourceData->GetSubresourceFootprints();
                const auto dataPointer = static_cast<uint8_t*>(resourceData->GetAllocation()->Map());
                std::array<uint8_t, 10> testBufferData = { 0xDE, 0xAD, 0xBE, 0xEF, 0x04, 0x08, 0x15, 0x16, 0x23, 0x42 };

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];
                    ASSERT(subresourceFootprint.width == subresourceFootprint.rowSizeInBytes);

                    auto columnPointer = reinterpret_cast<uint8_t*>(dataPointer);

                    for (uint32_t byte = 0; byte < subresourceFootprint.width; byte++)
                    {
                        *columnPointer = testBufferData[byte % testBufferData.size()];
                        columnPointer++;
                    }
                }
            }

            void initResourceData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& resourceData)
            {
                switch (description.GetFormat())
                {
                case GAPI::GpuResourceFormat::Unknown:
                    fillBufferData(description, resourceData);
                    break;
                case GAPI::GpuResourceFormat::RGBA8Uint:
                case GAPI::GpuResourceFormat::BGRA8Unorm:
                    fillColorData<uint32_t>(description, resourceData);
                    break;
                case GAPI::GpuResourceFormat::RGBA16Float:
                case GAPI::GpuResourceFormat::RGBA32Float:
                    fillColorData<Vector4>(description, resourceData);
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

            GAPI::GpuResourceDescription createTextureDescription(GAPI::GpuResourceDimension dimension, uint32_t size, GAPI::GpuResourceFormat format)
            {
                const auto numArraySlices = 3;

                switch (dimension)
                {
                case GAPI::GpuResourceDimension::Texture1D:
                    return GAPI::GpuResourceDescription::Texture1D(size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2D:
                    return GAPI::GpuResourceDescription::Texture2D(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2DMS:
                    return GAPI::GpuResourceDescription::Texture2DMS(size, size, format, 2, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::RenderTarget, numArraySlices);
                case GAPI::GpuResourceDimension::Texture3D:
                    return GAPI::GpuResourceDescription::Texture3D(size, size, size, format);
                case GAPI::GpuResourceDimension::TextureCube:
                    return GAPI::GpuResourceDescription::TextureCube(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, numArraySlices);
                }

                ASSERT_MSG(false, "Unsupported GpuResourceDimension");
                return GAPI::GpuResourceDescription::Texture1D(0, GAPI::GpuResourceFormat::Unknown, GAPI::GpuResourceBindFlags::ShaderResource);
            }

            GAPI::Buffer::SharedPtr initBufferWithData(const char* data, const GAPI::CopyCommandList::SharedPtr& commandList, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::ShaderResource)
            {
                auto& renderContext = Render::DeviceContext::Instance();

                const auto& description = GAPI::GpuResourceDescription::Buffer(strlen(data), bindFlags);
                const auto bufferData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);

                const auto dataPointer = static_cast<char*>(bufferData->GetAllocation()->Map());
                ON_SCOPE_EXIT(
                    {
                        bufferData->GetAllocation()->Unmap();
                    });

                const auto& footprint = bufferData->GetSubresourceFootprintAt(0);
                memcpy(dataPointer, data, footprint.rowSizeInBytes);

                auto result = renderContext.CreateBuffer(description, GAPI::GpuResourceCpuAccess::None, "Source");
                commandList->UpdateGpuResource(result, bufferData);

                return result;
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "ComputeCommmandList", "[CommandList][ComputeCommandList]")
        {
            auto& renderContext = Render::DeviceContext::Instance();

            auto commandList = renderContext.CreateCopyCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "ClearBufferUAV", "[CommandList][ComputeCommandList][ClearUAV]")
        {
            auto& renderContext = Render::DeviceContext::Instance();

            auto commandList = renderContext.CreateComputeCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            auto queue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Compute, "ComputeQueue");
            REQUIRE(queue != nullptr);

            const auto formatName = GAPI::GpuResourceFormatInfo::ToString(GAPI::GpuResourceFormat::Unknown);
            SECTION(fmt::format("[Buffer::{}] Copy buffer data on CPU", formatName))
            {
                const auto sourceData = "1234567890qwertyasdfg";
                const auto source = initBufferWithData(sourceData, commandList, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::UnorderedAccess);

                const auto testData = "QWE6789IOP";
                const auto readbackData = renderContext.AllocateIntermediateTextureData(source->GetDescription(), GAPI::MemoryAllocationType::Readback);

                const auto uav = source->GetUAV(GAPI::GpuResourceFormat::R32Uint, 4, 1);

                commandList->ClearUnorderedAccessViewUint(uav, Vector4u(61,64,255, 0xFFFF42));
                commandList->ReadbackGpuResource(source, readbackData);
                commandList->Close();

                submitAndWait(queue, commandList);

                const auto dataPointer = static_cast<uint8_t*>(readbackData->GetAllocation()->Map());
                ON_SCOPE_EXIT(
                    {
                        readbackData->GetAllocation()->Unmap();
                    });

                const auto& footprint = readbackData->GetSubresourceFootprintAt(0);
                REQUIRE(memcmp(dataPointer, testData, footprint.rowSizeInBytes) == 0);
            }
        }
    }
}