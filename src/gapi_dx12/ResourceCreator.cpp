#include "ResourceCreator.hpp"

#include "gapi_dx12/CommandContextImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"
#include "gapi_dx12/SwapChainImpl.hpp"
#include "gapi_dx12/TypeConversions.hpp"

#include "gapi/CommandContext.hpp"
#include "gapi/Fence.hpp"
#include "gapi/Object.hpp"
#include "gapi/RenderQueue.hpp"
#include "gapi/Resource.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            namespace
            {
                template <typename DescType>
                DescType getViewDimension(TextureDimension dimension, bool isTextureArray);

                template <>
                D3D12_RTV_DIMENSION getViewDimension(TextureDimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                    case TextureDimension::Texture1D:
                        return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
                    case TextureDimension::Texture2D:
                        return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
                    case TextureDimension::Texture3D:
                        ASSERT(isTextureArray == false);
                        return D3D12_RTV_DIMENSION_TEXTURE3D;
                    case TextureDimension::Texture2DMS:
                        return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    case TextureDimension::TextureCube:
                        return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    default:
                        ASSERT_MSG(false, "Wrong texture dimension")
                        return D3D12_RTV_DIMENSION_UNKNOWN;
                    }
                }

                template <>
                D3D12_DSV_DIMENSION getViewDimension(TextureDimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                    case TextureDimension::Texture1D:
                        return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE1DARRAY : D3D12_DSV_DIMENSION_TEXTURE1D;
                    case TextureDimension::Texture2D:
                        return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
                    case TextureDimension::Texture2DMS:
                        return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    case TextureDimension::TextureCube:
                        return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    default:
                        ASSERT_MSG(false, "Wrong texture dimension")
                        return D3D12_DSV_DIMENSION_UNKNOWN;
                    }
                }

                template <typename DescType>
                DescType CreateDsvRtvUavDescCommon(const TextureDescription& textureDescription, const ResourceViewDescription& description)
                {
                    DescType result = {};
                    result.ViewDimension = getViewDimension<decltype(result.ViewDimension)>(textureDescription.dimension, textureDescription.arraySize > 1);

                    const uint32_t arrayMultiplier = (textureDescription.dimension == TextureDimension::TextureCube) ? 6 : 1;
                    ASSERT((description.texture.firstArraySlice + description.texture.arraySlicesCount) * arrayMultiplier <= textureDescription.arraySize)

                    switch (textureDescription.dimension)
                    {
                    case TextureDimension::Texture1D:
                        if (description.texture.arraySlicesCount > 1)
                        {
                            result.Texture1DArray.ArraySize = description.texture.arraySlicesCount;
                            result.Texture1DArray.FirstArraySlice = description.texture.firstArraySlice;
                            result.Texture1DArray.MipSlice = description.texture.arraySlicesCount;
                        }
                        else
                        {
                            result.Texture1D.MipSlice = description.texture.firstArraySlice;
                        }
                        break;
                    case TextureDimension::Texture2D:
                    case TextureDimension::TextureCube:
                        if (description.texture.firstArraySlice * arrayMultiplier > 1)
                        {
                            result.Texture2DArray.ArraySize = description.texture.arraySlicesCount * arrayMultiplier;
                            result.Texture2DArray.FirstArraySlice = description.texture.firstArraySlice * arrayMultiplier;
                            result.Texture2DArray.MipSlice = description.texture.mipLevel;
                        }
                        else
                        {
                            result.Texture2D.MipSlice = description.texture.mipLevel;
                        }
                        break;
                    case TextureDimension::Texture2DMS:
                        //ASSERT(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value)
                        break;
                    default:
                        Log::Print::Fatal("Unsupported resource view type");
                    }
                    result.Format = TypeConversions::GetResourceFormat(textureDescription.format);

                    return result;
                }

                template <typename DescType>
                DescType CreateDsvRtvDesc(const TextureDescription& textureDescription, const ResourceViewDescription& description)
                {
                    static_assert(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value);

                    DescType result = CreateDsvRtvUavDescCommon<DescType>(textureDescription, description);

                    if (textureDescription.dimension == TextureDimension::Texture2DMS)
                    {
                        if (textureDescription.arraySize > 1)
                        {
                            result.Texture2DMSArray.ArraySize = description.texture.firstArraySlice;
                            result.Texture2DMSArray.FirstArraySlice = description.texture.arraySlicesCount;
                        }
                    }

                    return result;
                }

                D3D12_DEPTH_STENCIL_VIEW_DESC CreateDsvDesc(const Resource::SharedPtr& resource, const ResourceViewDescription& description)
                {
                    ASSERT(resource->GetResourceType() == Resource::ResourceType::Texture)
                    const auto& texture = resource->GetTyped<Texture>();
                    const auto& textureDescription = texture->GetDescription();

                    return CreateDsvRtvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(textureDescription, description);
                }

                D3D12_RENDER_TARGET_VIEW_DESC CreateRtvDesc(const Resource::SharedPtr& resource, const ResourceViewDescription& description)
                {
                    ASSERT(resource->GetResourceType() == Resource::ResourceType::Texture)
                    const auto& texture = resource->GetTyped<Texture>();
                    const auto& textureDescription = texture->GetDescription();

                    return CreateDsvRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(textureDescription, description);
                }

                Result initResource(const ResourceCreatorContext& context, Resource& resource)
                {
                    auto impl = new ResourceImpl();

                    switch (resource.GetResourceType())
                    {
                    case Resource::ResourceType::Texture:
                    {
                        const auto& texture = resource.GetTyped<Texture>();

                        D3DCall(impl->Init(context.device, texture->GetDescription(), texture->GetBindFlags(), resource.GetName()));
                    }
                    break;
                        //    case Resource::Type::Buffer:
                        //  D3DCall(impl->Init(context.device, resource.GetName()));
                        //       break;
                    default:
                        ASSERT_MSG(false, "Wrong resource type")
                        return Result::NotImplemented;
                    }

                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreatorContext& context, CommandQueue& resource)
                {
                    auto impl = new CommandQueueImpl(D3D12_COMMAND_LIST_TYPE_DIRECT);

                    D3DCall(impl->Init(context.device, resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreatorContext& context, ResourceView& object)
                {
                    const auto& resourceSharedPtr = object.GetResource().lock();
                    ASSERT(resourceSharedPtr);

                    const auto& resourcePrivateImpl = resourceSharedPtr->GetPrivateImpl<ResourceImpl>();
                    ASSERT(resourcePrivateImpl);

                    const auto& d3dObject = resourcePrivateImpl->getD3DObject();
                    ASSERT(d3dObject);

                    auto allocation = new DescriptorHeap::Allocation();
                    switch (object.GetViewType())
                    {
                    case ResourceView::ViewType::RenderTargetView:
                    {
                        const auto& descriptorHeap = context.descriptorHeapSet->GetRtvDescriptorHeap();
                        ASSERT(descriptorHeap);

                        D3DCall(descriptorHeap->Alloc(*allocation));

                        D3D12_RENDER_TARGET_VIEW_DESC desc = CreateRtvDesc(resourceSharedPtr, object.GetDescription());
                        context.device->CreateRenderTargetView(d3dObject.get(), &desc, allocation->GetCPUHandle());
                    }
                    break;
                        /*     case ResourceView::ViewType::RShaderResourceView:
                    break;
                case ResourceView::ViewType::RDepthStencilView:
                    break;
                case ResourceView::ViewType::RUnorderedAccessView:
                    break;*/
                    default:
                        LOG_FATAL("Unsupported resource view type");
                    }
                    object.SetPrivateImpl(allocation);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreatorContext& context, CommandContext& resource)
                {
                    auto impl = new CommandContextImpl();

                    D3DCall(impl->Init(context.device, resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreatorContext& context, uint64_t initialValue, Fence& resource)
                {
                    auto impl = new FenceImpl();

                    D3DCall(impl->Init(context.device, initialValue, resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreatorContext& context, SwapChain& resource)
                {
                    auto impl = new SwapChainImpl();

                    D3DCall(impl->Init(context.device, context.dxgiFactory, context.graphicsCommandQueue, resource.GetDescription(), resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }
            }

            Result ResourceCreator::InitResource(const ResourceCreatorContext& context, const Object::SharedPtr& resource)
            {
                ASSERT(resource)
                ASSERT(!resource->GetPrivateImpl<void*>())

#define CASE_RESOURCE(T)                                             \
    case Object::Type::T:                                            \
        result = initResource(context, dynamic_cast<T&>(*resource)); \
        break;

                Result result = Result::NotImplemented;

                switch (resource->GetType())
                {
                    CASE_RESOURCE(CommandContext)
                    CASE_RESOURCE(CommandQueue)
                    CASE_RESOURCE(Resource)
                    CASE_RESOURCE(ResourceView)
                    CASE_RESOURCE(SwapChain)
                default:
                    ASSERT_MSG(false, "Unsuported resource type");
                }
#undef CASE_RESOURCE

                if (!result)
                    Log::Print::Error("Error creating resource with eror: %s", result.ToString());

                return result;
            }

            Result ResourceCreator::InitResource(const ResourceCreatorContext& context, uint64_t initialValue, const Fence::SharedPtr& resource)
            {
                ASSERT(resource)
                ASSERT(!resource->GetPrivateImpl<void*>())

                Result result = initResource(context, initialValue, static_cast<Fence&>(*resource));

                if (!result)
                    Log::Print::Error("Error creating fence with eror: %s", result.ToString());

                return Result::Ok;
            }

        }
    }
}
