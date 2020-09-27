#include "ResourceCreator.hpp"

#include "gapi/RenderQueue.hpp"
#include "gapi/Resource.hpp"
#include "gapi/ResourceViews.hpp"

#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/RenderQueueImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            GAPIResult InitResource(ResourceCreatorContext& context, RenderQueue& resource)
            {
                auto impl = new RenderQueueImpl(D3D12_COMMAND_LIST_TYPE_DIRECT);

                D3DCall(impl->Init(context.device, resource.GetName()));
                resource.SetPrivateImpl(impl);

                return GAPIResult::OK;
            }

            GAPIResult InitResource(ResourceCreatorContext& context, RenderTargetView& resource)
            {
                auto allocation = new DescriptorHeap::Allocation();
                auto descriptorHeap = context.descriptorHeapSet->GetRtvDescriptorHeap();

                D3DCall(descriptorHeap->Alloc(*allocation));

                resource.SetPrivateImpl(allocation);

                return GAPIResult::OK;
            }

            GAPIResult ResourceCreator::InitResource(ResourceCreatorContext& context, Resource& resource)
            {
                ASSERT(!resource.GetPrivateImpl<void*>())

#define CASE_RESOURCE(T)    \
    case Resource::Type::T: \
        return InitResource(context, dynamic_cast<T&>(resource));

                switch (resource.GetType())
                {
                    CASE_RESOURCE(RenderQueue)
                    CASE_RESOURCE(RenderTargetView)
                }

                ASSERT_MSG(false, "Unsuported resource type");
#undef CASE_RESOURCE
            }

        }
    }
}
