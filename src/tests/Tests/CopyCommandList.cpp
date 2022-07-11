#include "CopyCommandList.hpp"

#include "TestContextFixture.hpp"

#include "ApprovalIntegration/BufferApprover.hpp"
#include "ApprovalIntegration/ImageApprover.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

#include "common/OnScopeExit.hpp"

namespace RR
{
    namespace Tests
    {
        TEST_CASE_METHOD(TestContextFixture, "CopyCommandList", "[CommandList][CopyCommandList]")
        {
            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyBuffer_old", "[CommandList][CopyCommandList][CopyBuffer]")
        {
            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto queue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
            REQUIRE(queue != nullptr);

            const auto format = GAPI::GpuResourceFormat::Unknown;
            const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer data on CPU", formatName))
            {
                const auto& description = GAPI::GpuResourceDescription::Buffer(128);

                const auto sourceData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                const auto destData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);

                initResourceData(description, sourceData);
                destData->CopyDataFrom(sourceData);

                REQUIRE(isResourceEqual(sourceData, destData));
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Upload buffer indirect", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);
                const auto& description = GAPI::BufferDescription::Buffer(128);

                const auto cpuData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                initResourceData(resdescription, cpuData);

                auto testBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                commandList->UpdateGpuResource(testBuffer, cpuData);
                commandList->ReadbackGpuResource(testBuffer, readbackData);
                commandList->Close();

                submitAndWait(queue, commandList);
                REQUIRE(isResourceEqual(cpuData, readbackData));
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Upload buffer direct", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);
                const auto& description = GAPI::BufferDescription::Buffer(128);

                const auto cpuData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                const auto sourceData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Upload);
                const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                initResourceData(resdescription, cpuData);
                sourceData->CopyDataFrom(cpuData);

                auto testTexture = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                commandList->UpdateGpuResource(testTexture, sourceData);
                commandList->ReadbackGpuResource(testTexture, readbackData);
                commandList->Close();

                submitAndWait(queue, commandList);
                REQUIRE(isResourceEqual(cpuData, readbackData));
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer on GPU", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);
                const auto& description = GAPI::BufferDescription::Buffer(128);

                const auto sourceData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                initResourceData(resdescription, sourceData);

                auto source = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Source");
                auto dest = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                commandList->UpdateGpuResource(source, sourceData);
                commandList->CopyGpuResource(source, dest);
                commandList->ReadbackGpuResource(dest, readbackData);

                commandList->Close();

                submitAndWait(queue, commandList);
                REQUIRE(isResourceEqual(sourceData, readbackData));
            }

            {
                const auto format = GAPI::GpuResourceFormat::Unknown;
                const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

                DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy structured buffer on GPU", formatName))
                {
                    struct TestStuct
                    {
                        uint32_t a1;
                        float a2;
                        bool a3;
                    };

                    const auto& resdescription = GAPI::GpuResourceDescription::StructuredBuffer(128, sizeof(TestStuct));
                    const auto& description = GAPI::BufferDescription::StructuredBuffer(sizeof(TestStuct) * 128, sizeof(TestStuct));

                    const auto sourceData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                    const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                    initResourceData(resdescription, sourceData);

                    auto source = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Source");
                    auto dest = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                    commandList->UpdateGpuResource(source, sourceData);
                    commandList->CopyGpuResource(source, dest);
                    commandList->ReadbackGpuResource(dest, readbackData);

                    commandList->Close();

                    submitAndWait(queue, commandList);
                    REQUIRE(isResourceEqual(sourceData, readbackData));
                }

                DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer region", formatName))
                {
                    const auto sourceData = "1234567890";
                    auto source = createBufferWithData(sourceData, "Source");

                    const auto destData = "QWERTYUIOP";
                    auto dest = createBufferWithData(destData, "Dest");

                    const auto testData = "QWE6789IOP";

                    const auto& description = GAPI::GpuResourceDescription::Buffer(strlen(destData));
                    const auto readbackData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::Readback);

                    commandList->CopyBufferRegion(source, 5, dest, 3, 4);
                    commandList->ReadbackGpuResource(dest, readbackData);
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

        TEST_CASE_METHOD(TestContextFixture, "CopyBuffer", "[CommandList][CopyCommandList][CopyBuffer]")
        {
            auto commandList = renderContext.CreateCopyCommandList(u8"CopyCommandList");
            REQUIRE(commandList != nullptr);

            auto queue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Copy, "CopyQueue");
            REQUIRE(queue != nullptr);

            const auto format = GAPI::GpuResourceFormat::Unknown;
            const auto formatName = GAPI::GpuResourceFormatInfo::ToString(format);

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Initial data", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);
                auto& description = GAPI::BufferDescription::Buffer(128);

                description.usage = GAPI::GpuResourceUsage::Upload;
                auto uploadBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Upload, "UploadBuffer");

                description.usage = GAPI::GpuResourceUsage::Default;
                auto gpuBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Readback, "ReadbackBuffer");

                initResourceData(uploadBuffer);

                commandList->CopyGpuResource(uploadBuffer, gpuBuffer);
                commandList->CopyGpuResource(gpuBuffer, readbackBuffer);
                commandList->Close();

                submitAndWait(queue, commandList);
                BufferApprover::verify(readbackBuffer);
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Upload buffer", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);

                auto& description = GAPI::BufferDescription::Buffer(128);

                description.usage = GAPI::GpuResourceUsage::Upload;
                auto uploadBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Upload, "UploadBuffer");

                description.usage = GAPI::GpuResourceUsage::Default;
                auto gpuBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Readback, "ReadbackBuffer");

                initResourceData(uploadBuffer);

                commandList->CopyGpuResource(uploadBuffer, gpuBuffer);
                commandList->CopyGpuResource(gpuBuffer, readbackBuffer);
                commandList->Close();

                submitAndWait(queue, commandList);
                BufferApprover::verify(readbackBuffer);
            }

            DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer on GPU", formatName))
            {
                const auto& resdescription = GAPI::GpuResourceDescription::Buffer(128);
                auto& description = GAPI::BufferDescription::Buffer(128);

                description.usage = GAPI::GpuResourceUsage::Upload;
                auto uploadBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Upload, "UploadBuffer");

                description.usage = GAPI::GpuResourceUsage::Default;
                auto sourceBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Source");
                auto destBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                description.usage = GAPI::GpuResourceUsage::Readback;
                auto readbackBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Readback, "ReadbackBuffer");

                initResourceData(uploadBuffer);

                commandList->CopyGpuResource(uploadBuffer, sourceBuffer);
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
                        uint32_t a1;
                        float a2;
                        bool a3;
                    };

                    const auto& resdescription = GAPI::GpuResourceDescription::StructuredBuffer(128, sizeof(TestStuct));
                    auto& description = GAPI::BufferDescription::StructuredBuffer(sizeof(TestStuct) * 128, sizeof(TestStuct));

                    description.usage = GAPI::GpuResourceUsage::Upload;
                    const auto uploadBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Upload, "UploadBuffer");

                    description.usage = GAPI::GpuResourceUsage::Default;
                    const auto sourceBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Source");
                    const auto destBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                    description.usage = GAPI::GpuResourceUsage::Readback;
                    const auto readbackBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Readback, "ReadbackBuffer");

                    initResourceData(uploadBuffer);

                    commandList->CopyGpuResource(uploadBuffer, sourceBuffer);
                    commandList->CopyGpuResource(sourceBuffer, destBuffer);
                    commandList->CopyGpuResource(destBuffer, readbackBuffer);

                    commandList->Close();

                    submitAndWait(queue, commandList);
                    BufferApprover::verify(readbackBuffer);
                }

                DYNAMIC_SECTION(fmt::format("[Buffer::{}] Copy buffer region", formatName))
                {
                    const auto sourceData = "1234567890";
                    auto source = createBufferWithData(sourceData, "Source");

                    const auto destData = "QWERTYUIOP";
                    auto dest = createBufferWithData(destData, "Dest");

                    const auto testData = "QWE6789IOP";

                    const auto& resdescription = GAPI::GpuResourceDescription::Buffer(strlen(destData));
                    auto& description = GAPI::BufferDescription::Buffer(strlen(destData));

                    description.usage = GAPI::GpuResourceUsage::Readback;
                    const auto readbackBuffer = renderContext.CreateBuffer(resdescription, description, nullptr, GAPI::GpuResourceUsage::Readback, "ReadbackBuffer");

                    commandList->CopyBufferRegion(source, 5, dest, 3, 4);
                    commandList->CopyGpuResource(dest, readbackBuffer);
                    commandList->Close();

                    submitAndWait(queue, commandList);

                    const auto resouceData = GAPI::GpuResourceDataGuard(readbackBuffer);

                    const auto& footprint = readbackBuffer->GetSubresourceFootprints()[0];
                    REQUIRE(memcmp(resouceData.Data(), testData, footprint.rowSizeInBytes) == 0);
                }
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "CopyTexture", "[CommandList][CopyCommandList][CopyTexture]")
        {
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
                        const auto description = createResTexDescription(dimension, 128, format);

                        const auto sourceData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto destData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);

                        initResourceData(description, sourceData);
                        destData->CopyDataFrom(sourceData);

                        REQUIRE(isResourceEqual(sourceData, destData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Upload texure indirect", dimensionTitle, formatName))
                    {
                        const auto description = createResTexDescription(dimension, 128, format);
                        const auto texDescription = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto cpuData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto readbackData = renderContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::Readback);

                        initResourceData(description, cpuData);

                        auto testTexture = renderContext.CreateTexture(description, texDescription, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                        commandList->UpdateGpuResource(testTexture, cpuData);
                        commandList->ReadbackGpuResource(testTexture, readbackData);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(cpuData, readbackData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Upload texure direct", dimensionTitle, formatName))
                    {
                        const auto resdescription = createResTexDescription(dimension, 128, format);
                        const auto description = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto cpuData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto sourceData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Upload);
                        const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                        initResourceData(resdescription, cpuData);
                        sourceData->CopyDataFrom(cpuData);

                        auto testTexture = renderContext.CreateTexture(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

                        commandList->UpdateGpuResource(testTexture, sourceData);
                        commandList->ReadbackGpuResource(testTexture, readbackData);
                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(cpuData, readbackData));
                    }

                    DYNAMIC_SECTION(fmt::format("[{}::{}] Copy texure on GPU", dimensionTitle, formatName))
                    {
                        const auto resdescription = createResTexDescription(dimension, 128, format);
                        const auto description = createTextureDescription(dimension, 128, format, GAPI::GpuResourceUsage::Default);

                        const auto sourceData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        const auto readbackData = renderContext.AllocateIntermediateResourceData(resdescription, GAPI::MemoryAllocationType::Readback);

                        initResourceData(resdescription, sourceData);

                        auto source = renderContext.CreateTexture(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Source");
                        auto dest = renderContext.CreateTexture(resdescription, description, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                        commandList->UpdateGpuResource(source, sourceData);
                        commandList->CopyGpuResource(source, dest);
                        commandList->ReadbackGpuResource(dest, readbackData);

                        commandList->Close();

                        submitAndWait(copyQueue, commandList);
                        REQUIRE(isResourceEqual(sourceData, readbackData));
                    }
                    /*
                    DYNAMIC_SECTION(fmt::format("[{}::{}] CopyTextureSubresource", dimensionTitle, formatName))
                    {
                        const auto sourceDescription = createTextureDescription(dimension, 256, format);
                        const auto sourceData = renderContext.AllocateIntermediateResourceData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        auto source = renderContext.CreateTexture(sourceDescription, nullptr, GAPI::GpuResourceUsage::Default, "Source");

                        initResourceData(sourceDescription, sourceData);
                        commandList->UpdateGpuResource(source, sourceData);

                        const auto destDescription = createTextureDescription(dimension, 128, format);
                        const auto destData = renderContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        auto dest = renderContext.CreateTexture(destDescription, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                        initResourceData(destDescription, destData);
                        commandList->UpdateGpuResource(dest, destData);

                        for (uint32_t index = 0; index < destDescription.GetNumSubresources(); index++)
                        {
                            const auto mipLevel = destDescription.GetSubresourceMipLevel(index);
                            const auto arraySlice = destDescription.GetSubresourceArraySlice(index);
                            const auto face = destDescription.GetSubresourceFace(index);

                            if (mipLevel % 2 != 0)
                                commandList->CopyTextureSubresource(source, sourceDescription.GetSubresourceIndex(arraySlice, mipLevel + 1, face), dest, index);
                        }

                        const auto readbackData = renderContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Readback);
                        commandList->ReadbackGpuResource(dest, readbackData);
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
                    }*/
                }
                /*
                DYNAMIC_SECTION(fmt::format("[Texture3D::{}] CopyTextureSubresource", formatName))
                {
                    const auto sourceDescription = createTextureDescription(GAPI::GpuResourceDimension::Texture3D, 128, format);
                    const auto sourceData = renderContext.AllocateIntermediateResourceData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                    auto source = renderContext.CreateTexture(sourceDescription, nullptr, GAPI::GpuResourceUsage::Default, "Source");

                    initResourceData(sourceDescription, sourceData);
                    commandList->UpdateGpuResource(source, sourceData);

                    const auto destDescription = createTextureDescription(GAPI::GpuResourceDimension::Texture3D, 64, format);
                    const auto destData = renderContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Upload);
                    auto dest = renderContext.CreateTexture(destDescription, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                    initResourceData(destDescription, destData);
                    commandList->UpdateGpuResource(dest, destData);

                    commandList->CopyTextureSubresourceRegion(source, 1, Box3u(7, 8, 13, 32, 32, 32), dest, 0, Vector3u(16, 16, 16));
                    commandList->CopyTextureSubresourceRegion(source, 2, Box3u(0, 0, 0, 16, 16, 16), dest, 1, Vector3u(8, 8, 8));
                    commandList->CopyTextureSubresourceRegion(source, 0, Box3u(3, 42, 66, 8, 8, 8), dest, 2, Vector3u(0, 0, 0));

                    const auto readbackData = renderContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Readback);
                    commandList->ReadbackGpuResource(dest, readbackData);
                    commandList->Close();

                    submitAndWait(copyQueue, commandList);
                    ImageApprover::verify(readbackData);
                }*/
            }
        }
    }
}