#include "ComputeCommandList.hpp"

#include "TestContextFixture.hpp"

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
        TEST_CASE_METHOD(TestContextFixture, "ComputeCommmandList", "[CommandList][ComputeCommandList]")
        {
            auto commandList = deviceContext.CreateCopyCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            SECTION("Close")
            {
                commandList->Close();
            }
        }

        TEST_CASE_METHOD(TestContextFixture, "ClearBufferUAV", "[CommandList][ComputeCommandList][ClearUAV]")
        {
            auto commandList = deviceContext.CreateComputeCommandList(u8"ComputeCommmandList");
            REQUIRE(commandList != nullptr);

            auto queue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Compute, "ComputeQueue");
            REQUIRE(queue != nullptr);

            SECTION("[Buffer::RawBuffer] UAV clear ClearUnorderedAccessViewUint")
            {
                const auto sourceData = "1234567890qwertyasdfg";
                const auto source = createBufferFromString(sourceData, "Source", GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::UnorderedAccess);

                auto readbackDesc = GAPI::GpuResourceDescription::Buffer(
                    source->GetDescription().buffer.size,
                    GAPI::GpuResourceBindFlags::None,
                    GAPI::GpuResourceUsage::Readback);
                const auto readback = deviceContext.CreateBuffer(readbackDesc, nullptr, "Readback");

                const auto uav = source->GetUAV(GAPI::GpuResourceFormat::R32Uint);

                commandList->ClearUnorderedAccessViewUint(uav, Vector4u(0x61626365, 13, 8, 64));
                commandList->CopyGpuResource(source, readback);
                commandList->Close();

                submitAndWait(queue, commandList);

                auto data = GAPI::GpuResourceDataGuard(readback);
                const auto testData = "ecbaecbaecbaecbaecbag";
                REQUIRE(memcmp(data.Data(), testData, strlen(testData)) == 0);
            }

            SECTION(fmt::format("[Buffer::RawBuffer] Partical ClearUnorderedAccessViewFloat"))
            {
                const auto sourceData = "1234567890qwertyasdfg";
                const auto source = createBufferFromString(sourceData, "Source", GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::UnorderedAccess);

                auto readbackDesc = GAPI::GpuResourceDescription::Buffer(
                    source->GetDescription().buffer.size,
                    GAPI::GpuResourceBindFlags::None,
                    GAPI::GpuResourceUsage::Readback);
                const auto readback = deviceContext.CreateBuffer(readbackDesc, nullptr, "Readback");

                const auto uav = source->GetUAV(GAPI::GpuResourceFormat::R32Uint, 1, 1);

                commandList->ClearUnorderedAccessViewFloat(uav, Vector4(0x23232323, 13, 8, 64));
                commandList->CopyGpuResource(source, readback);
                commandList->Close();

                submitAndWait(queue, commandList);

                const auto testData = "1234@###90qwertyasdfg";
                auto data = GAPI::GpuResourceDataGuard(readback);
                REQUIRE(memcmp(data.Data(), testData, strlen(testData)) == 0);
            }

            SECTION("[Texture2D::RGBA8] Partical ClearUnorderedAccessViewUint")
            {
                auto description = GAPI::GpuResourceDescription::Texture2D(128, 128, GAPI::GpuResourceFormat::RGBA8Uint, GAPI::GpuResourceBindFlags::UnorderedAccess);

                auto source = deviceContext.CreateTexture(description, createTestColorData(description), "Source");

                description.usage = GAPI::GpuResourceUsage::Readback;
                const auto readback = deviceContext.CreateTexture(description, nullptr, "Source");

                const auto uav = source->GetUAV(1);

                commandList->ClearUnorderedAccessViewUint(uav, Vector4u(128, 255, 64, 255));
                commandList->CopyGpuResource(source, readback);
                commandList->Close();

                submitAndWait(queue, commandList);
                ImageApprover::verify(readback);
            }
        }
    }
}