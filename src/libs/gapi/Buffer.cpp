#pragma once

#include "Buffer.hpp"

#include "gapi/GpuResourceViews.hpp"

#include "render/DeviceContext.hpp"

#include "common/Math.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            const GpuResourceViewDescription& createViewDescription(const GpuResourceDescription& resourceDesc, GpuResourceFormat format, uint32_t firstElement, uint32_t numElements)
            {
                const auto elementSize = GpuResourceFormatInfo::GetBlockSize(format);

                //Dx12 requerement
                ASSERT(firstElement % 4 == 0);
                ASSERT(firstElement * elementSize < resourceDesc.GetNumElements());

                if (numElements == Buffer::MaxPossible)
                    numElements = (resourceDesc.GetNumElements() - firstElement) / 4;

                ASSERT((firstElement + numElements) * elementSize < resourceDesc.GetNumElements());

                return GpuResourceViewDescription::Buffer(format, firstElement, numElements);
            }
        }

        ShaderResourceView::SharedPtr Buffer::GetSRV(GpuResourceFormat format, uint32_t firstElement, uint32_t numElements)
        {
            const auto viewDesc = createViewDescription(description_, format, firstElement, numElements);

            if (srvs_.find(viewDesc) == srvs_.end())
            {
                auto& renderContext = Render::DeviceContext::Instance();
                // TODO static_pointer_cast; name_
                srvs_[viewDesc] = renderContext.CreateShaderResourceView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return srvs_[viewDesc];
        }

        UnorderedAccessView::SharedPtr Buffer::GetUAV(GpuResourceFormat format, uint32_t firstElement, uint32_t numElements)
        {
            const auto viewDesc = createViewDescription(description_, format, firstElement, numElements);

            if (uavs_.find(viewDesc) == uavs_.end())
            {
                auto& renderContext = Render::DeviceContext::Instance();
                // TODO static_pointer_cast; name_
                uavs_[viewDesc] = renderContext.CreateUnorderedAccessView(std::static_pointer_cast<Buffer>(shared_from_this()), viewDesc);
            }

            return uavs_[viewDesc];
        }
    }
}