#pragma once

#include "Buffer.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        Buffer::Buffer(const GpuResourceDescription& description, GpuResourceCpuAccess cpuAccess, const U8String& name)
            : GpuResource(description, cpuAccess, name)
        {
            //TODO asserts
        }

        ShaderResourceView::SharedPtr Buffer::GetSRV(uint32_t firstElement, uint32_t numElements)
        {
            //TODO asserts
            ASSERT(firstElement < description_.width);
            ASSERT(numElements == MaxPossible || firstElement + numElements < description_.width);

            numElements = Min(numElements, description_.width - firstElement);
            const auto& viewDesc = GpuResourceViewDescription(firstElement, numElements);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                srvs_[viewDesc] = renderContext.CreateShaderResourceView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return srvs_[viewDesc];
        }

        UnorderedAccessView::SharedPtr Buffer::GetUAV(uint32_t firstElement, uint32_t numElements)
        {
            //TODO asserts
            ASSERT(firstElement < description_.width);
            ASSERT(numElements == MaxPossible || firstElement + numElements < description_.width);

            numElements = Min(numElements, description_.width - firstElement);
            const auto& viewDesc = GpuResourceViewDescription(firstElement, numElements);

            if (uavs_.find(viewDesc) == uavs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                uavs_[viewDesc] = renderContext.CreateUnorderedAccessView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return uavs_[viewDesc];
        }
    }
}