#include "ResourceCreator.hpp"

#include "gapi/Object.hpp"
#include "gapi/RenderQueue.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/CommandContext.hpp"

#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/RenderQueueImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"
#include "gapi_dx12/CommandContextImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            /*
            template <typename DescType, bool finalCall>
            DescType CreateDsvRtvUavDescCommon(const Resource* pResource)
            {
                DescType desc = {};
                const Texture* pTexture = dynamic_cast<const Texture*>(pResource);
                assert(pTexture); // Buffers should not get here

                desc = {};
                uint32_t arrayMultiplier = (pResource->getType() == Resource::Type::TextureCube) ? 6 : 1;

                if (arraySize == Resource::kMaxPossible)
                {
                    arraySize = pTexture->getArraySize() - firstArraySlice;
                }

                desc.ViewDimension = getViewDimension<decltype(desc.ViewDimension)>(pTexture->getType(), pTexture->getArraySize() > 1);

                switch (pResource->getType())
                {
                case Resource::Type::Texture1D:
                    if (pTexture->getArraySize() > 1)
                    {
                        desc.Texture1DArray.ArraySize = arraySize;
                        desc.Texture1DArray.FirstArraySlice = firstArraySlice;
                        desc.Texture1DArray.MipSlice = mipLevel;
                    }
                    else
                    {
                        desc.Texture1D.MipSlice = mipLevel;
                    }
                    break;
                case Resource::Type::Texture2D:
                case Resource::Type::TextureCube:
                    if (pTexture->getArraySize() * arrayMultiplier > 1)
                    {
                        desc.Texture2DArray.ArraySize = arraySize * arrayMultiplier;
                        desc.Texture2DArray.FirstArraySlice = firstArraySlice * arrayMultiplier;
                        desc.Texture2DArray.MipSlice = mipLevel;
                    }
                    else
                    {
                        desc.Texture2D.MipSlice = mipLevel;
                    }
                    break;
                default:
                    if (finalCall)
                        should_not_get_here();
                }
                desc.Format = getDxgiFormat(pTexture->getFormat());

                return desc;
            }

            template <typename DescType>
            DescType CreateDsvRtvDesc(const Resource* pResource)
            {
                DescType desc = createDsvRtvUavDescCommon<DescType, false>(pResource);

                if (pResource->getType() == Resource::Type::Texture2DMultisample)
                {
                    const Texture* pTexture = dynamic_cast<const Texture*>(pResource);
                    if (pTexture->getArraySize() > 1)
                    {
                        desc.Texture2DMSArray.ArraySize = arraySize;
                        desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
                    }
                }

                return desc;
            }

            D3D12_DEPTH_STENCIL_VIEW_DESC CreateDsvDesc(const Resource* pResource)
            {
                return CreateDsvRtvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(pResource);
            }

            D3D12_RENDER_TARGET_VIEW_DESC CreateRtvDesc(const Resource* pResource)
            {
                return CreateDsvRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(pResource);
            }
            */
            Result InitResource(ResourceCreatorContext& context, RenderQueue& resource)
            {
                auto impl = new RenderQueueImpl(D3D12_COMMAND_LIST_TYPE_DIRECT);

                D3DCall(impl->Init(context.device, resource.GetName()));
                resource.SetPrivateImpl(impl);

                return Result::Ok;
            }

            Result InitResource(ResourceCreatorContext& context, ResourceView& resource)
            { /*
                auto allocation = new DescriptorHeap::Allocation();
                auto descriptorHeap = context.descriptorHeapSet->GetRtvDescriptorHeap();
          

                
                D3DCall(descriptorHeap->Alloc(*allocation));

                D3D12_RENDER_TARGET_VIEW_DESC = CreateRtvDesc();
                context.device->CreateRenderTargetView(render, desc, allocation->GetCPUHandle())

                    resource.SetPrivateImpl(allocation);*/

                return Result::Ok;
            }

            Result InitResource(ResourceCreatorContext& context, CommandContext& resource)
            {
                auto impl = new CommandContextImpl();

                D3DCall(impl->Init(context.device, resource.GetName()));
                resource.SetPrivateImpl(impl);

                return Result::Ok;
            }

            Result ResourceCreator::InitResource(ResourceCreatorContext& context, Object::ConstSharedPtrRef resource)
            {
                ASSERT(resource)
                ASSERT(!resource->GetPrivateImpl<void*>())

#define CASE_RESOURCE(T)  \
    case Object::Type::T: \
        return InitResource(context, dynamic_cast<T&>(*resource));

                switch (resource->GetType())
                {
                    CASE_RESOURCE(RenderQueue)
                    CASE_RESOURCE(ResourceView)
                    CASE_RESOURCE(CommandContext)
                }

                ASSERT_MSG(false, "Unsuported resource type");
#undef CASE_RESOURCE

                return Result::Ok;
            }

        }
    }
}
