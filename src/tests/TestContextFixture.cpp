#include "TestContextFixture.hpp"

#include "render/DeviceContext.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "common/DataBuffer.hpp"
#include "common/OnScopeExit.hpp"

namespace RR
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
            IDataBuffer::SharedPtr getColorData(const GAPI::GpuResourceDescription& description)
            {
                const auto& deviceContext = Render::DeviceContext::Instance();

                ASSERT((std::is_same<T, uint32_t>::value && description.format == GAPI::GpuResourceFormat::RGBA8Uint) ||
                       (std::is_same<T, uint32_t>::value && description.format == GAPI::GpuResourceFormat::BGRA8Unorm) ||
                       (std::is_same<T, Vector4>::value && description.format == GAPI::GpuResourceFormat::RGBA16Float) ||
                       (std::is_same<T, Vector4>::value && description.format == GAPI::GpuResourceFormat::RGBA32Float));

                const auto footprint = deviceContext.GetResourceFootprint(description);
                const auto buffer = std::make_shared<DataBuffer>(footprint.totalSize);

                const auto& subresourceFootprints = footprint.subresourceFootprints;
                const auto blockSize = GAPI::GpuResourceFormatInfo::GetBlockSize(description.format);

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];
                    ASSERT(subresourceFootprint.width * blockSize == subresourceFootprint.rowSizeInBytes);

                    for (uint32_t depth = 0; depth < subresourceFootprint.depth; depth++)
                    {
                        const auto depthPointer = static_cast<std::byte*>(buffer->Data()) + subresourceFootprint.offset +
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

                return buffer;
            }

            template <typename T>
            void fillColorData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& resourceData)
            {
                ASSERT(resourceData->GetFirstSubresource() == 0);
                ASSERT((std::is_same<T, uint32_t>::value && description.format == GAPI::GpuResourceFormat::RGBA8Uint) ||
                       (std::is_same<T, uint32_t>::value && description.format == GAPI::GpuResourceFormat::BGRA8Unorm) ||
                       (std::is_same<T, Vector4>::value && description.format == GAPI::GpuResourceFormat::RGBA16Float) ||
                       (std::is_same<T, Vector4>::value && description.format == GAPI::GpuResourceFormat::RGBA32Float));

                const auto& subresourceFootprints = resourceData->GetSubresourceFootprints();
                const auto dataPointer = static_cast<uint8_t*>(resourceData->GetAllocation()->Map());
                const auto blockSize = GAPI::GpuResourceFormatInfo::GetBlockSize(description.format);

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
                ASSERT(description.dimension == GAPI::GpuResourceDimension::Buffer);

                const auto& subresourceFootprints = resourceData->GetSubresourceFootprints();
                const auto dataPointer = static_cast<uint8_t*>(resourceData->GetAllocation()->Map());
                std::array<uint8_t, 10> testBufferData = { 0xDE, 0xAD, 0xBE, 0xEF, 0x04, 0x08, 0x15, 0x16, 0x23, 0x42 };

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];
                    ASSERT(subresourceFootprint.width * description.buffer.stride == subresourceFootprint.rowSizeInBytes);

                    auto columnPointer = reinterpret_cast<uint8_t*>(dataPointer);

                    for (uint32_t byte = 0; byte < subresourceFootprint.width; byte++)
                    {
                        *columnPointer = testBufferData[byte % testBufferData.size()];
                        columnPointer++;
                    }
                }
            }

            void fillBufferData(const GAPI::GpuResource::SharedPtr& resource)
            {
                ASSERT(resource->GetDescription().IsBuffer());

                const auto& description = resource->GetDescription();
                const auto resourceData = GAPI::GpuResourceDataGuard(resource);

                const auto& subresourceFootprints = resource->GetSubresourceFootprints();

                std::array<uint8_t, 10> testBufferData = { 0xDE, 0xAD, 0xBE, 0xEF, 0x04, 0x08, 0x15, 0x16, 0x23, 0x42 };

                for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
                {
                    const auto& subresourceFootprint = subresourceFootprints[index];
                    ASSERT(subresourceFootprint.width == subresourceFootprint.rowSizeInBytes);

                    auto subresourcePointer = reinterpret_cast<uint8_t*>(resourceData.Data()) + subresourceFootprint.offset;

                    for (size_t byte = 0; byte < subresourceFootprint.width; byte++)
                    {
                        *subresourcePointer = testBufferData[byte % testBufferData.size()];
                        subresourcePointer++;
                    }
                }
            }
        }

        TestContextFixture::TestContextFixture() : deviceContext(Render::DeviceContext::Instance())
        {
        }

        GAPI::GpuResourceDescription TestContextFixture::createTextureDescription(GAPI::GpuResourceDimension dimension, uint32_t size, GAPI::GpuResourceFormat format, GAPI::GpuResourceUsage usage)
        {
            const auto numArraySlices = 3;

            switch (dimension)
            {
                case GAPI::GpuResourceDimension::Texture1D: return GAPI::GpuResourceDescription::Texture1D(size, format, GAPI::GpuResourceBindFlags::ShaderResource, usage, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2D: return GAPI::GpuResourceDescription::Texture2D(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, usage, numArraySlices);
                case GAPI::GpuResourceDimension::Texture2DMS: return GAPI::GpuResourceDescription::Texture2DMS(size, size, format, GAPI::MultisampleType::MSAA_2, GAPI::GpuResourceBindFlags::ShaderResource | GAPI::GpuResourceBindFlags::RenderTarget, usage, numArraySlices);
                case GAPI::GpuResourceDimension::Texture3D: return GAPI::GpuResourceDescription::Texture3D(size, size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, usage);
                case GAPI::GpuResourceDimension::TextureCube: return GAPI::GpuResourceDescription::TextureCube(size, size, format, GAPI::GpuResourceBindFlags::ShaderResource, usage, numArraySlices);
            }

            ASSERT_MSG(false, "Unsupported GpuResourceDimension");
            return GAPI::GpuResourceDescription::Texture1D(0, GAPI::GpuResourceFormat::Unknown, GAPI::GpuResourceBindFlags::ShaderResource);
        }

        void TestContextFixture::initResourceData(const GAPI::GpuResource::SharedPtr& resource)
        {
            if (resource->GetDescription().IsBuffer())
            {
                fillBufferData(resource);
            }
            else
            {
            }
        }

        void TestContextFixture::initTextureData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& resourceData)
        {
            switch (description.format)
            {
                case GAPI::GpuResourceFormat::Unknown: fillBufferData(description, resourceData); break;
                case GAPI::GpuResourceFormat::RGBA8Uint:
                case GAPI::GpuResourceFormat::BGRA8Unorm: fillColorData<uint32_t>(description, resourceData); break;
                case GAPI::GpuResourceFormat::RGBA16Float:
                case GAPI::GpuResourceFormat::RGBA32Float: fillColorData<Vector4>(description, resourceData); break;
                default: LOG_FATAL("Unsupported format");
            }
        }

        GAPI::Buffer::SharedPtr TestContextFixture::createBufferFromString(const char* data, const U8String& name, GAPI::GpuResourceBindFlags bindFlags)
        {
            const auto dataBuffer = std::make_shared<DataBuffer>(strlen(data), static_cast<const void*>(data));
            const auto description = GAPI::GpuResourceDescription::Buffer(dataBuffer->Size(), GAPI::GpuResourceFormat::Unknown, bindFlags);

            return deviceContext.CreateBuffer(description, dataBuffer, name);
        }

        IDataBuffer::SharedPtr TestContextFixture::createTestColorData(const GAPI::GpuResourceDescription& description)
        {
            switch (description.format)
            {
                case GAPI::GpuResourceFormat::RGBA8Uint:
                case GAPI::GpuResourceFormat::BGRA8Unorm:
                    return getColorData<uint32_t>(description);
                case GAPI::GpuResourceFormat::RGBA16Float:
                case GAPI::GpuResourceFormat::RGBA32Float:
                    return getColorData<Vector4>(description);
                default: LOG_FATAL("Unsupported format"); return nullptr;
            }
        }

        bool TestContextFixture::isDataEqual(const GAPI::GpuResourceDescription& description,
                                             const std::shared_ptr<IDataBuffer>& lhs,
                                             const std::shared_ptr<IDataBuffer>& rhs)
        {
            ASSERT(lhs != rhs);
            const auto footprint = deviceContext.GetResourceFootprint(description);

            for (const auto& subresourceFootprint : footprint.subresourceFootprints)
                if (!isSubresourceEqual(subresourceFootprint, lhs, rhs))
                    return false;

            return true;
        }

        bool TestContextFixture::isResourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs,
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

        bool TestContextFixture::isSubresourceEqual(const GAPI::CpuResourceData::SubresourceFootprint& footprint,
                                                    const std::shared_ptr<IDataBuffer>& lhs,
                                                    const std::shared_ptr<IDataBuffer>& rhs)
        {
            ASSERT(lhs);
            ASSERT(rhs);
            ASSERT(lhs != rhs);

            auto lrowPointer = static_cast<std::byte*>(lhs->Data()) + footprint.offset;
            auto rrowPointer = static_cast<std::byte*>(rhs->Data()) + footprint.offset;

            for (uint32_t row = 0; row < footprint.numRows; row++)
            {
                if (memcmp(lrowPointer, rrowPointer, footprint.rowSizeInBytes) != 0)
                    return false;

                lrowPointer += footprint.rowPitch;
                rrowPointer += footprint.rowPitch;
            }

            return true;
        }

        bool TestContextFixture::isSubresourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs, uint32_t lSubresourceIndex,
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

        void TestContextFixture::submitAndWait(const std::shared_ptr<GAPI::CommandQueue>& commandQueue, const std::shared_ptr<GAPI::CommandList>& commandList)
        {
            deviceContext.Submit(commandQueue, commandList);
            deviceContext.WaitForGpu(commandQueue);
            deviceContext.MoveToNextFrame(commandQueue);
        }
    }
}