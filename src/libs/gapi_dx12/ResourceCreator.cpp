#include "ResourceCreator.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/DeviceContext.hpp"
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
                    ASSERT((description.texture.firstArraySlice + description.texture.arraySliceCount) * arrayMultiplier <= textureDescription.arraySize)

                    switch (textureDescription.dimension)
                    {
                    case TextureDimension::Texture1D:
                        if (description.texture.arraySliceCount > 1)
                        {
                            result.Texture1DArray.ArraySize = description.texture.arraySliceCount;
                            result.Texture1DArray.FirstArraySlice = description.texture.firstArraySlice;
                            result.Texture1DArray.MipSlice = description.texture.mipLevel;
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
                            result.Texture2DArray.ArraySize = description.texture.arraySliceCount * arrayMultiplier;
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
                            result.Texture2DMSArray.FirstArraySlice = description.texture.arraySliceCount;
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

                template <typename T, typename Impl>
                void releaseResource(T& resource)
                {
                    static_assert(std::is_base_of<Object, T>::value, "T should be derived from Object");

                    if (!resource.GetPrivateImpl())
                        return;

                    const auto impl = resource.GetPrivateImpl<Impl>();
                    ASSERT(impl);

                    impl->ReleaseD3DObjects();

                    resource.SetPrivateImpl(nullptr);
                }

                template <>
                void releaseResource<GpuResourceView, DescriptorHeap::Allocation>(GpuResourceView& resource)
                {
                    const auto impl = resource.GetPrivateImpl<DescriptorHeap::Allocation>();
                    ASSERT(impl);

                    // Todo delete?

                    resource.SetPrivateImpl(nullptr);
                }
            }

            Result ResourceCreator::InitSwapChain(SwapChain& resource)
            {
                auto& deviceContext = DeviceContext::Instance();

                auto impl = std::make_unique<SwapChainImpl>();
                D3DCall(impl->Init(deviceContext.GetDevice(), deviceContext.GetDxgiFactory(), deviceContext.GetGraphicsCommandQueue()->GetD3DObject(), resource.GetDescription(), resource.GetName()));

                resource.SetPrivateImpl(impl.release());

                return Result::Ok;
            }

            Result ResourceCreator::InitFence(Fence& resource)
            {
                auto impl = std::make_unique<FenceImpl>();
                D3DCall(impl->Init(resource.GetName()));

                resource.SetPrivateImpl(impl.release());

                return Result::Ok;
            }

            Result ResourceCreator::InitCommandQueue(CommandQueue& resource)
            {
                std::unique_ptr<CommandQueueImpl> impl;
                auto& deviceContext = DeviceContext::Instance();

                if (resource.GetCommandQueueType() == CommandQueueType::Graphics)
                {
                    static bool alreadyInited = false;
                    ASSERT(!alreadyInited); // Only one graphics command queue are alloved.
                    alreadyInited = true;

                    // Graphics command queue already initialized internally in device,
                    // so make copy to prevent d3d object leaking.
                    impl.reset(new CommandQueueImpl(*deviceContext.GetGraphicsCommandQueue()));
                }
                else
                {
                    impl.reset(new CommandQueueImpl(resource.GetCommandQueueType()));
                    D3DCall(impl->Init(resource.GetName()));
                }

                resource.SetPrivateImpl(impl.release());

                return Result::Ok;
            }

            Result ResourceCreator::InitCommandList(CommandList& resource)
            {
                auto impl = std::make_unique<CommandListImpl>(resource.GetCommandListType());
                const auto result = impl->Init(resource.GetName());

                resource.SetPrivateImpl(static_cast<IGraphicsCommandList*>(impl.release()));

                return Result::Ok;
            }

            Result ResourceCreator::InitBuffer(Buffer& resource)
            {
                auto& deviceContext = DeviceContext::Instance();

                auto impl = std::make_unique<ResourceImpl>();
                //   const auto result = impl->Init(deviceContext.GetDevice(), resource.GetDescription(), resource.GetBindFlags(), resource.GetName());
                /*
                if (!result)
                {
                    delete impl;
                    LOG_ERROR("Error creating Buffer with error: %s", result.ToString());

                    return result;
                }*/

                //Buffer   resource.SetPrivateImpl(impl);

                return Result::Ok;
            }

            Result ResourceCreator::InitGpuResourceView(GpuResourceView& object)
            {
                auto& deviceContext = DeviceContext::Instance();

                const auto& resourceSharedPtr = object.GetGpuResource().lock();
                ASSERT(resourceSharedPtr);

                const auto resourcePrivateImpl = resourceSharedPtr->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourcePrivateImpl);

                const auto& d3dObject = resourcePrivateImpl->GetD3DObject();
                ASSERT(d3dObject);

                auto allocation = std::make_unique<DescriptorHeap::Allocation>();

                switch (object.GetViewType())
                {
                case GpuResourceView::ViewType::RenderTargetView:
                {
                    const auto& descriptorHeap = deviceContext.GetDesciptorHeapSet()->GetRtvDescriptorHeap();
                    ASSERT(descriptorHeap);

                    D3DCall(descriptorHeap->Allocate(*allocation));

                    D3D12_RENDER_TARGET_VIEW_DESC desc = CreateRtvDesc(resourceSharedPtr, object.GetDescription());
                    deviceContext.GetDevice()->CreateRenderTargetView(d3dObject.get(), &desc, allocation->GetCPUHandle());
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

                object.SetPrivateImpl(allocation.release());

                return Result::Ok;
            }

            void ResourceCreator::ReleaseResource(Object& resource)
            {
#define CASE_RESOURCE(T, IMPL)                               \
    case Object::Type::T:                                    \
        ASSERT(dynamic_cast<T*>(&resource));                 \
        releaseResource<T, IMPL>(static_cast<T&>(resource)); \
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