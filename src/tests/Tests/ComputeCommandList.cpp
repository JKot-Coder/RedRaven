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

#include "common/OnScopeExit.hpp"

namespace OpenDemo
{
    namespace Tests
    {
        TEST_CASE_METHOD( TestContextFixture, "ComputeCommmandList", "[CommandList][ComputeCommandList]")
        {
            auto commandList = renderContext.CreateCopyCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD( TestContextFixture, "ClearBufferUAV", "[CommandList][ComputeCommandList][ClearUAV]")
        {
            auto commandList = renderContext.CreateComputeCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            auto queue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Compute, "ComputeQueue");
            REQUIRE(queue != nullptr);

            SECTION(fmt::format("[Buffer::RawBuffer] UAV clear"))
            {
                const auto sourceData = "1234567890qwertyasdfg";
                const auto source = initBufferWithData(sourceData, commandList, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::UnorderedAccess);
                const auto readbackData = renderContext.AllocateIntermediateTextureData(source->GetDescription(), GAPI::MemoryAllocationType::Readback);

                const auto uav = source->GetUAV(GAPI::GpuResourceFormat::R32Uint);

                commandList->ClearUnorderedAccessViewUint(uav, Vector4u(0x61626365, 13, 8, 64));
                commandList->ReadbackGpuResource(source, readbackData);
                commandList->Close();

                submitAndWait(queue, commandList);

                const auto dataPointer = static_cast<uint8_t*>(readbackData->GetAllocation()->Map());
                ON_SCOPE_EXIT(
                    {
                        readbackData->GetAllocation()->Unmap();
                    });

                const auto testData = "ecbaecbaecbaecbaecbag";
                const auto& footprint = readbackData->GetSubresourceFootprintAt(0);
                REQUIRE(memcmp(dataPointer, testData, footprint.rowSizeInBytes) == 0);
            }

            SECTION(fmt::format("[Buffer::RawBuffer] Partical UAV clear"))
            {
                const auto sourceData = "1234567890qwertyasdfg";
                const auto source = initBufferWithData(sourceData, commandList, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::UnorderedAccess);
                const auto readbackData = renderContext.AllocateIntermediateTextureData(source->GetDescription(), GAPI::MemoryAllocationType::Readback);

                const auto uav = source->GetUAV(GAPI::GpuResourceFormat::R32Uint, 4, 1);

                commandList->ClearUnorderedAccessViewUint(uav, Vector4u(0x61626365, 13, 8, 64));
                commandList->ReadbackGpuResource(source, readbackData);
                commandList->Close();

                submitAndWait(queue, commandList);

                const auto dataPointer = static_cast<uint8_t*>(readbackData->GetAllocation()->Map());
                ON_SCOPE_EXIT(
                    {
                        readbackData->GetAllocation()->Unmap();
                    });

                const auto testData = "1234567890qwertyecbag";
                const auto& footprint = readbackData->GetSubresourceFootprintAt(0);
                REQUIRE(memcmp(dataPointer, testData, footprint.rowSizeInBytes) == 0);
            }
        }
    }
}