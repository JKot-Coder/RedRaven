#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/GpuResource.hpp"

namespace RR
{
    namespace Tests
    {
        class TestContextFixture
        {
        public:
            TestContextFixture();
            ~TestContextFixture() = default;

        protected:
            GAPI::GpuResourceDescription createTextureDescription(GAPI::GpuResourceDimension dimension, uint32_t size, GAPI::GpuResourceFormat format);
            void initResourceData(const GAPI::GpuResourceDescription& description, const std::shared_ptr<GAPI::CpuResourceData>& resourceData);
            void initResourceData(const GAPI::GpuResource::SharedPtr& resource);
            std::shared_ptr<GAPI::Buffer> createBufferWithData(const char* data, const U8String& name, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::ShaderResource);

            bool isResourceEqual(const std::shared_ptr<GAPI::CpuResourceData>& lhs, const std::shared_ptr<GAPI::CpuResourceData>& rhs);
            bool isSubresourceEqual(const std::shared_ptr<GAPI::CpuResourceData>& lhs, uint32_t lSubresourceIndex,
                                    const std::shared_ptr<GAPI::CpuResourceData>& rhs, uint32_t rSubresourceIndex);

            void submitAndWait(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList);

        protected:
            Render::DeviceContext& renderContext;
        };
    }
}