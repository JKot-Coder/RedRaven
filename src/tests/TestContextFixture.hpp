#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/GpuResource.hpp"

namespace RR
{
    namespace Common
    {
        class IDataBuffer;
    }

    namespace Tests
    {
        class TestContextFixture
        {
        public:
            TestContextFixture();
            ~TestContextFixture() = default;

        protected:
            GAPI::GpuResourceDescription createTextureDescription(GAPI::GpuResourceDimension dimension, uint32_t size, GAPI::GpuResourceFormat format, GAPI::GpuResourceUsage usage);
            std::shared_ptr<GAPI::Buffer> createBufferFromString(const char* data, const U8String& name, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::ShaderResource);
            std::shared_ptr<IDataBuffer> createTestColorData(const GAPI::GpuResourceDescription& description);

            bool isDataEqual(const GAPI::GpuResourceDescription& description, const std::shared_ptr<IDataBuffer>& lhs, const std::shared_ptr<IDataBuffer>& rhs);
            bool isSubresourceEqual(const GAPI::GpuResourceFootprint::SubresourceFootprint& lhsFootprint,
                                    const std::shared_ptr<IDataBuffer>& lhs,
                                    const GAPI::GpuResourceFootprint::SubresourceFootprint& rhsFootprint,
                                    const std::shared_ptr<IDataBuffer>& rhs);

            void submitAndWait(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList);

        protected:
            Render::DeviceContext& deviceContext;
        };
    }
}