#include "ResourceCreator.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"
#include "gapi_dx12/SwapChainImpl.hpp"
#include "gapi_dx12/TypeConversions.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/Object.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace GAPI
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
                        ASSERT_MSG(false, "Wrong texture dimension");
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
                DescType CreateDsvRtvUavDescCommon(const TextureDescription& textureDescription, const GpuResourceViewDescription& description)
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
                    result.Format = TypeConversions::GetGpuResourceFormat(textureDescription.format);

                    return result;
                }

                template <typename DescType>
                DescType CreateDsvRtvDesc(const TextureDescription& textureDescription, const GpuResourceViewDescription& description)
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

                D3D12_DEPTH_STENCIL_VIEW_DESC CreateDsvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    ASSERT(resource->GetGpuResourceType() == GpuResource::Type::Texture)
                    const auto& texture = resource->GetTyped<Texture>();
                    const auto& textureDescription = texture->GetDescription();

                    return CreateDsvRtvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(textureDescription, description);
                }

                D3D12_RENDER_TARGET_VIEW_DESC CreateRtvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    ASSERT(resource->GetGpuResourceType() == GpuResource::Type::Texture)
                    const auto& texture = resource->GetTyped<Texture>();
                    const auto& textureDescription = texture->GetDescription();

                    return CreateDsvRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(textureDescription, description);
                }

                Result initResource(const ResourceCreateContext& context, GpuResource& resource)
                {
                    auto impl = new ResourceImpl();

                    switch (resource.GetGpuResourceType())
                    {
                    case GpuResource::Type::Texture:
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

                Result initResource(const ResourceCreateContext& context, CommandQueue& resource)
                {
                    CommandQueueImpl* impl = nullptr;

                    if (resource.GetCommandQueueType() == CommandQueueType::Graphics)
                    {
                        static bool alreadyInited = false;
                        ASSERT(!alreadyInited); // Only one graphics command queue are alloved.
                        ASSERT(context.graphicsCommandQueue);
                        alreadyInited = true;

                        // Graphics command queue already initialized internally in device,
                        // so make copy to prevent d3d object leaking.
                        impl = new CommandQueueImpl(*context.graphicsCommandQueue.get());
                    }
                    else
                    {
                        impl = new CommandQueueImpl(resource.GetCommandQueueType());
                        D3DCall(impl->Init(context.device, resource.GetName()));
                    }

                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreateContext& context, GpuResourceView& object)
                {
                    const auto& resourceSharedPtr = object.GetGpuResource().lock();
                    ASSERT(resourceSharedPtr);

                    const auto resourcePrivateImpl = resourceSharedPtr->GetPrivateImpl<ResourceImpl>();
                    ASSERT(resourcePrivateImpl);

                    const auto& d3dObject = resourcePrivateImpl->GetD3DObject();
                    ASSERT(d3dObject);

                    auto allocation = new DescriptorHeap::Allocation();
                    switch (object.GetViewType())
                    {
                    case GpuResourceView::ViewType::RenderTargetView:
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

                Result initResource(const ResourceCreateContext& context, CommandList& resource)
                {
                    auto impl = new CommandListImpl(resource.GetCommandListType());

                    D3DCall(impl->Init(context.device, resource.GetName()));
                    resource.SetPrivateImpl(static_cast<IGraphicsCommandList*>(impl));

                    return Result::Ok;
                }

                Result initResource(const ResourceCreateContext& context, Fence& resource)
                {
                    auto impl = new FenceImpl();

                    D3DCall(impl->Init(context.device, resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                Result initResource(const ResourceCreateContext& context, SwapChain& resource)
                {
                    auto impl = new SwapChainImpl();

                    ASSERT(context.graphicsCommandQueue->GetD3DObject());

                    D3DCall(impl->Init(context.device, context.dxgiFactory, context.graphicsCommandQueue->GetD3DObject(), resource.GetDescription(), resource.GetName()));
                    resource.SetPrivateImpl(impl);

                    return Result::Ok;
                }

                template <typename T, typename Impl>
                void releaseResource(ResourceReleaseContext& resourceReleaseContext, T& resource)
                {
                    static_assert(std::is_base_of<Object, T>::value, "T should be derived from Object");

                    if (!resource.GetPrivateImpl())
                        return;

                    const auto impl = resource.GetPrivateImpl<Impl>();
                    ASSERT(impl);

                    impl->ReleaseD3DObjects(resourceReleaseContext);

                    resource.SetPrivateImpl(nullptr);
                }

                template <>
                void releaseResource<GpuResourceView, DescriptorHeap::Allocation>(ResourceReleaseContext& resourceReleaseContext, GpuResourceView& resource)
                {
                    const auto impl = resource.GetPrivateImpl<DescriptorHeap::Allocation>();
                    ASSERT(impl);

                    // Todo delete?

                    resource.SetPrivateImpl(nullptr);
                }
            }

            Result ResourceCreator::InitResource(const ResourceCreateContext& context, const Object::SharedPtr& resource)
            {
                ASSERT(resource)

#define CASE_RESOURCE(T)                                                        \
    case Object::Type::T:                                                       \
        ASSERT(std::dynamic_pointer_cast<T>(resource));                         \
        ASSERT(!std::static_pointer_cast<T>(resource)->GetPrivateImpl());       \
        result = initResource(context, *std::static_pointer_cast<T>(resource)); \
        break;

                Result result = Result::NotImplemented;

                switch (resource->GetType())
                {
                    CASE_RESOURCE(CommandList)
                    CASE_RESOURCE(CommandQueue)
                    CASE_RESOURCE(Fence)
                    CASE_RESOURCE(GpuResource)
                    CASE_RESOURCE(GpuResourceView)
                    CASE_RESOURCE(SwapChain)
                default:
                    ASSERT_MSG(false, "Unsuported resource type");
                }

#undef CASE_RESOURCE

                if (!result)
                    Log::Print::Error("Error creating resource with eror: %s", result.ToString());

                return result;
            }

            void ResourceCreator::ReleaseResource(ResourceReleaseContext& resourceReleaseContext, Object& resource)
            {
#define CASE_RESOURCE(T, IMPL)                                                       \
    case Object::Type::T:                                                            \
        ASSERT(dynamic_cast<T*>(&resource));                                         \
        releaseResource<T, IMPL>(resourceReleaseContext, static_cast<T&>(resource)); \
        break;

                switch (resource.GetType())
                {
                    CASE_RESOURCE(CommandList, CommandListImpl)
                    CASE_RESOURCE(CommandQueue, CommandQueueImpl)
                    CASE_RESOURCE(Fence, FenceImpl)
                    CASE_RESOURCE(GpuResource, ResourceImpl)
                    CASE_RESOURCE(GpuResourceView, DescriptorHeap::Allocation)
                    CASE_RESOURCE(SwapChain, SwapChainImpl)
                default:
                    ASSERT_MSG(false, "Unsuported resource type");
                }

#undef CASE_RESOURCE
            }
        }
    }
}