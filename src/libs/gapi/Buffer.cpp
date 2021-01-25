#pragma once

#include "Buffer.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/RenderContext.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        Buffer::Buffer(const BufferDescription& desc, GpuResourceBindFlags bindFlags, const U8String& name)
            : GpuResource(GpuResource::Type::Buffer, bindFlags, name),
              description_(desc)
        {
        }

        ShaderResourceView::SharedPtr Buffer::GetSRV(uint32_t firstElement, uint32_t numElements)
        {
            ASSERT(firstElement < description_.size);
            ASSERT(numElements == MaxPossible || firstElement + numElements < description_.size);

            numElements = Min(numElements, description_.size - firstElement);
            const auto& viewDesc = GpuResourceViewDescription(firstElement, numElements);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                srvs_[viewDesc] = renderContext.CreateShaderResourceView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc, name_);
            }

            return srvs_[viewDesc];
        }

        UnorderedAccessView::SharedPtr Buffer::GetUAV(uint32_t firstElement, uint32_t numElements)
        {
            ASSERT(firstElement < description_.size);
            ASSERT(numElements == MaxPossible || firstElement + numElements < description_.size);

            numElements = Min(numElements, description_.size - firstElement);
            const auto& viewDesc = GpuResourceViewDescription(firstElement, numElements);

            if (uavs_.find(viewDesc) == uavs_.end())
            {
                auto& renderContext = Render::RenderContext::Instance();
                // TODO static_pointer_cast; name_
                uavs_[viewDesc] = renderContext.CreateUnorderedAccessView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc, name_);
            }

            return uavs_[viewDesc];
        }
    }
}