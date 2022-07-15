#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include "ApprovalIntegration/BufferApprover.hpp"
#include "ApprovalIntegration/ImageApprover.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

#include "common/OnScopeExit.hpp"

namespace RR
{
    namespace Tests
    {
        TEST_CASE_METHOD(TestContextFixture, "CopyCommandList", "[CommandList][CopyCommandList]")
        {
            auto commandList = deviceContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyBuffer", "[CommandList][CopyCommandList][CopyBuffer]")
        {
            auto commandList = deviceContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto queue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
            REQUIRE(queue != nullptr);

            const auto format = GAPI::GpuResourceFormat::Unknown;
            const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Initial data", formatName))
            {
                const auto testString = "Initial data: DEABC844";
                const auto dataBuffer = std::make_shared<DataBuffer>(strlen(testString), static_cast<const void*>(testString));
                auto description = GAPI::GpuResourceDescription::Buffer(dataBuffer->Size());

                description.usage = GAPI::GpuResourceUsage::Default;
                auto gpuBuffer = deviceContext.CreateBuffer(description, dataBuffer, "Test");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = deviceContext.CreateBuffer(description, nullptr, "ReadbackBuffer");

                commandList->CopyGpuResource(gpuBuffer, readbackBuffer);
                commandList->Close();

                submitAndWait(queue, commandList);
                BufferApprover::verify(readbackBuffer);
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Upload buffer", formatName))
            {
                const auto testString = "Initial data: 2A9E274E";
                const auto dataBuffer = std::make_shared<DataBuffer>(strlen(testString), static_cast<const void*>(testString));
                auto description = GAPI::GpuResourceDescription::Buffer(dataBuffer->Size());

                description.usage = GAPI::GpuResourceUsage::Upload;
                auto uploadBuffer = deviceContext.CreateBuffer(description, dataBuffer, "UploadBuffer");

                description.usage = GAPI::GpuResourceUsage::Default;
                auto gpuBuffer = deviceContext.CreateBuffer(description, nullptr, "Test");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = deviceContext.CreateBuffer(description, nullptr, "ReadbackBuffer");

                commandList->CopyGpuResource(uploadBuffer, gpuBuffer);
                commandList->CopyGpuResource(gpuBuffer, readbackBuffer);
                commandList->Close();

                submitAndWait(queue, commandList);
                BufferApprover::verify(readbackBuffer);
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer on GPU", formatName))
            {
                const auto testString = "Initial data: B8549709";
                const auto dataBuffer = std::make_shared<DataBuffer>(strlen(testString), static_cast<const void*>(testString));
                auto description = GAPI::GpuResourceDescription::Buffer(dataBuffer->Size());

                auto sourceBuffer = deviceContext.CreateBuffer(description, dataBuffer, "Source");
                auto destBuffer = deviceContext.CreateBuffer(description, nullptr, "Dest");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = deviceContext.CreateBuffer(description, nullptr, "ReadbackBuffer");

                commandList->CopyGpuResource(sourceBuffer, destBuffer);
                commandList->CopyGpuResource(destBuffer, readbackBuffer);

                commandList->Close();

                submitAndWait(queue, commandList);
                BufferApprover::verify(readbackBuffer);
            }

            {
                const auto format = GAPI::GpuResourceFormat::Unknown;
                const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

                DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy structured buffer on GPU", formatName))
                {
                    struct TestStuct
                    {
                        char c3[10] = "Test data";
                        uint8_t p1[2] = { 0, 0 };
                        uint32_t a1 = 0xDEADBEEF;
                        bool b2 = true;
                        uint8_t p2[3] = { 0, 0, 0 };
                    };
                    static_assert(sizeof(TestStuct) == 20);

                    constexpr uint32_t numOfStructs = 6;
                    const TestStuct data[numOfStructs];

                    auto description = GAPI::GpuResourceDescription::StructuredBuffer(numOfStructs, sizeof(TestStuct));
                    const auto dataBuffer = std::make_shared<DataBuffer>(sizeof(TestStuct) * numOfStructs, static_cast<const void*>(data));

                    const auto sourceBuffer = deviceContext.CreateBuffer(description, dataBuffer, "Source");
                    const auto destBuffer = deviceContext.CreateBuffer(description, nullptr, "Dest");

                    description.usage = GAPI::GpuResourceUsage::Readback;
                    const auto readbackBuffer = deviceContext.CreateBuffer(description, nullptr, "ReadbackBuffer");

                    commandList->CopyGpuResource(sourceBuffer, destBuffer);
                    commandList->CopyGpuResource(destBuffer, readbackBuffer);

                    commandList->Close();

                    submitAndWait(queue, commandList);
                    BufferApprover::verify(readbackBuffer);
                }

                DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer region", formatName))
                {
                    const auto sourceData = "1234567890";
                    auto source = createBufferFromString(sourceData, "Source");

                    const auto destData = "QWERTYUIOP";
                    auto dest = createBufferFromString(destData, "Dest");

                    const auto testData = "QWE6789IOP";

                    auto& description = GAPI::GpuResourceDescription::Buffer(strlen(destData));

                    description.usage = GAPI::GpuResourceUsage::Readback;
                    const auto readbackBuffer = deviceContext.CreateBuffer(description, nullptr, "ReadbackBuffer");

                    commandList->CopyBufferRegion(source, 5, dest, 3, 4);
                    commandList->CopyGpuResource(dest, readbackBuffer);
                    commandList->Close();

                    submitAndWait(queue, commandList);

                    const auto resouceData = GAPI::GpuResourceDataGuard(readbackBuffer);

                    const auto footprint = readbackBuffer->GetSubresourceFootprints()[0];
                    REQUIRE(memcmp(resouceData.Data(), testData, footprint.rowSizeInBytes) == 0);
                }
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyTexture", "[CommandList][CopyCommandList][CopyTexture]")
        {
            auto commandList = deviceContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto copyQueue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
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

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Initial data", dimensionTitle, formatName))
                    {
                        auto description = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto initialData = createTestColorData(description);
                        auto source = deviceContext.CreateTexture(description, initialData, "Source");

                        description.usage = GAPI::GpuResourceUsage::Readback;
                        auto readback = deviceContext.CreateTexture(description, nullptr, "Readback");

                        commandList->CopyGpuResource(source, readback);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isDataEqual(description, initialData, std::make_shared<GAPI::GpuResourceDataGuard>(readback)));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Upload texure", dimensionTitle, formatName))
                    {
                        auto description = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto initialData = createTestColorData(description);
                        auto source = deviceContext.CreateTexture(description, initialData, "Source");

                        description.usage = GAPI::GpuResourceUsage::Upload;
                        auto upload = deviceContext.CreateTexture(description, initialData, "Upload");

                        description.usage = GAPI::GpuResourceUsage::Readback;
                        auto readback = deviceContext.CreateTexture(description, nullptr, "Readback");

                        commandList->CopyGpuResource(upload, source);
                        commandList->CopyGpuResource(source, readback);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isDataEqual(description, initialData, std::make_shared<GAPI::GpuResourceDataGuard>(readback)));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Copy texure on GPU", dimensionTitle, formatName))
                    {
                        auto description = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto initialData = createTestColorData(description);
                        auto source = deviceContext.CreateTexture(description, initialData, "Source");
                        auto dest = deviceContext.CreateTexture(description, nullptr, "Dest");

                        description.usage = GAPI::GpuResourceUsage::Readback;
                        auto readback = deviceContext.CreateTexture(description, nullptr, "Readback");

                        commandList->CopyGpuResource(source, dest);
                        commandList->CopyGpuResource(dest, readback);

                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isDataEqual(description, initialData, std::make_shared<GAPI::GpuResourceDataGuard>(readback)));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] CopyTextureSubresource", dimensionTitle, formatName))
                    {
                        const auto sourceDescription = createTextureDescription(dimension, 256, format, GAPI::GpuResourceUsage::Default);
                        const auto sourceData = createTestColorData(sourceDescription);
                        auto source = deviceContext.CreateTexture(sourceDescription, sourceData, "Source");

                        const auto destDescription = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);
                        const auto destData = createTestColorData(destDescription);
                        auto dest = deviceContext.CreateTexture(destDescription, destData, "Dest");

                        auto readbackDescription = destDescription;
                        readbackDescription.usage = GAPI::GpuResourceUsage::Readback;
                        auto readback = deviceContext.CreateTexture(readbackDescription, nullptr, "Readback");

                        for (uint32_t index = 0; index < destDescription.GetNumSubresources(); index++)
                        {
                            const auto mipLevel = destDescription.GetSubresourceMipLevel(index);
                            const auto arraySlice = destDescription.GetSubresourceArraySlice(index);
                            const auto face = destDescription.GetSubresourceFace(index);

                            if (mipLevel % 2 != 0)
                                commandList->CopyTextureSubresource(source, sourceDescription.GetSubresourceIndex(arraySlice, mipLevel + 1, face), dest, index);
                        }

                        commandList->CopyGpuResource(dest, readback);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);

                        const auto& sourceFootprints = source->GetSubresourceFootprints();
                        const auto& destFootprints = dest->GetSubresourceFootprints();
                        const auto& readbackFootprints = readback->GetSubresourceFootprints();

                        for (uint32_t index = 0; index < readbackDescription.GetNumSubresources(); index++)
                        {
                            const auto mipLevel = readbackDescription.GetSubresourceMipLevel(index);
                            const auto arraySlice = readbackDescription.GetSubresourceArraySlice(index);
                            const auto face = readbackDescription.GetSubresourceFace(index);

                            bool equal = (mipLevel % 2 != 0) ? isSubresourceEqual(sourceFootprints[sourceDescription.GetSubresourceIndex(arraySlice, mipLevel + 1, face)],
                                                                                  sourceData,
                                                                                  readbackFootprints[index],
                                                                                  std::make_shared<GAPI::GpuResourceDataGuard>(readback))
                                                             : isSubresourceEqual(destFootprints[index],
                                                                                  destData,
                                                                                  readbackFootprints[index],
                                                                                  std::make_shared<GAPI::GpuResourceDataGuard>(readback));
                            REQUIRE(equal);
                        }
                    }
                }
                /*
                DYNAMIC_SECTION(fmt::format("[Texture3D::{}] CopyTextureSubresource", formatName))
                {
                    const auto sourceDescription = createTextureDescription(GAPI::GpuResourceDimension::Texture3D, 128, format);
                    const auto sourceData = deviceContext.AllocateIntermediateResourceData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                    auto source = deviceContext.CreateTexture(sourceDescription, nullptr, GAPI::GpuResourceUsage::Default, "Source");

                    initResourceData(sourceDescription, sourceData);
                    commandList->UpdateGpuResource(source, sourceData);

                    const auto destDescription = createTextureDescription(GAPI::GpuResourceDimension::Texture3D, 64, format);
                    const auto destData = deviceContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Upload);
                    auto dest = deviceContext.CreateTexture(destDescription, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                    initResourceData(destDescription, destData);
                    commandList->UpdateGpuResource(dest, destData);

                    commandList->CopyTextureSubresourceRegion(source, 1, Box3u(7, 8, 13, 32, 32, 32), dest, 0, Vector3u(16, 16, 16));
                    commandList->CopyTextureSubresourceRegion(source, 2, Box3u(0, 0, 0, 16, 16, 16), dest, 1, Vector3u(8, 8, 8));
                    commandList->CopyTextureSubresourceRegion(source, 0, Box3u(3, 42, 66, 8, 8, 8), dest, 2, Vector3u(0, 0, 0));

                    const auto readbackData = deviceContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Readback);
                    commandList->ReadbackGpuResource(dest, readbackData);
                    commandList->Close();

                    submitAndWait(copyQueue, commandList);
                    ImageApprover::verify(readbackData);
                }*/
            }
        }
    }
}