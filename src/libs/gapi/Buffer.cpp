#include "Buffer.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/DeviceContext.hpp"

#include "math/Math.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            GpuResourceViewDescription createViewDescription(const GpuResourceDescription& resourceDesc, GpuResourceFormat format, size_t firstElement, size_t numElements)
            {
                uint32_t elementSize = 1;

                if (format != GpuResourceFormat::Unknown)
                {
                    elementSize = GpuResourceFormatInfo::GetBlockSize(format);
                }
                else if (resourceDesc.buffer.stride > 0)
                {
                    elementSize = resourceDesc.buffer.stride;
                }

                ASSERT(firstElement * elementSize < resourceDesc.buffer.size);

                if (numElements == Buffer::MaxPossible)
                    numElements = (resourceDesc.buffer.size / elementSize - firstElement);

                ASSERT((firstElement + numElements) * elementSize < resourceDesc.buffer.size);
                return GpuResourceViewDescription::Buffer(format, firstElement, numElements);
            }
        }

        const ShaderResourceView* Buffer::GetSRV(GpuResourceFormat format, size_t firstElement, size_t numElements)
        {
            const auto viewDesc = createViewDescription(description_, format, firstElement, numElements);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& deviceContext = RenderLoom::DeviceContext::Instance();
                // TODO static_pointer_cast; name_ !!!!!!!!!!!
                ///srvs_[viewDesc] = deviceContext.CreateShaderResourceView(eastl::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return srvs_[viewDesc].get();
        }

        const UnorderedAccessView* Buffer::GetUAV(GpuResourceFormat format, size_t firstElement, size_t numElements)
        {
            const auto& viewDesc = createViewDescription(description_, format, firstElement, numElements);

            if (uavs_.find(viewDesc) == uavs_.end())
            {
                auto& deviceContext = RenderLoom::DeviceContext::Instance();
                // TODO static_pointer_cast; name_
               // uavs_[viewDesc] = deviceContext.CreateUnorderedAccessView(eastl::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return uavs_[viewDesc].get();
        }
    }
}