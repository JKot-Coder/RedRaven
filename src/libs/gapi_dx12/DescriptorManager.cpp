#include "DescriptorManager.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceImpl.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                template <typename DescType>
                DescType getViewDimension(TextureDescription::Dimension dimension, bool isTextureArray);

                template <>
                D3D12_RTV_DIMENSION getViewDimension(TextureDescription::Dimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case TextureDescription::Dimension::Texture1D: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
                        case TextureDescription::Dimension::Texture2D: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
                        case TextureDescription::Dimension::Texture3D: ASSERT(isTextureArray == false); return D3D12_RTV_DIMENSION_TEXTURE3D;
                        case TextureDescription::Dimension::Texture2DMS: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                        case TextureDescription::Dimension::TextureCube: return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_RTV_DIMENSION_UNKNOWN;
                }

                template <>
                D3D12_DSV_DIMENSION getViewDimension(TextureDescription::Dimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case TextureDescription::Dimension::Texture1D: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE1DARRAY : D3D12_DSV_DIMENSION_TEXTURE1D;
                        case TextureDescription::Dimension::Texture2D: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
                        case TextureDescription::Dimension::Texture2DMS: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                        case TextureDescription::Dimension::TextureCube: return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_DSV_DIMENSION_UNKNOWN;
                }

                template <>
                D3D12_UAV_DIMENSION getViewDimension(TextureDescription::Dimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case TextureDescription::Dimension::Texture1D: return (isTextureArray) ? D3D12_UAV_DIMENSION_TEXTURE1DARRAY : D3D12_UAV_DIMENSION_TEXTURE1D;
                        case TextureDescription::Dimension::Texture2D: return (isTextureArray) ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D;
                        case TextureDescription::Dimension::TextureCube: return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_UAV_DIMENSION_UNKNOWN;
                }

                D3D12_UNORDERED_ACCESS_VIEW_DESC createUavBufferDesc(const BufferDescription& bufferDesc, const GpuResourceViewDescription& viewDesc)
                {
                    D3D12_UNORDERED_ACCESS_VIEW_DESC description;

                    description.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                    description.Buffer.StructureByteStride = bufferDesc.stride;
                    description.Buffer.CounterOffsetInBytes = 0;

                    /* if (!bufferDesc.IsTyped())
                    {
                        description.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
                    }*/
                    description.Format = D3DUtils::GetDxgiTypelessFormat(viewDesc.format);
                    description.Buffer.FirstElement = viewDesc.buffer.firstElement;
                    description.Buffer.NumElements = viewDesc.buffer.elementCount;

                    return description;
                }

                template <typename DescType>
                DescType createDsvRtvUavDescCommon(const TextureDescription& textureDesc, const GpuResourceViewDescription& viewDesc)
                {
                    DescType result = {};

                    const uint32_t arraySize = textureDesc.arraySize;
                    const uint32_t arrayMultiplier = (textureDesc.dimension == TextureDescription::Dimension::TextureCube) ? 6 : 1;

                    ASSERT((viewDesc.texture.firstArraySlice + viewDesc.texture.arraySliceCount) * arrayMultiplier <= arraySize);

                    result.ViewDimension = getViewDimension<decltype(result.ViewDimension)>(textureDesc.dimension, arraySize > 1);
                    result.Format = D3DUtils::GetDxgiResourceFormat(textureDesc.format);

                    switch (textureDesc.dimension)
                    {
                        case TextureDescription::Dimension::Texture1D:
                            if (viewDesc.texture.arraySliceCount > 1)
                            {
                                result.Texture1DArray.ArraySize = viewDesc.texture.arraySliceCount;
                                result.Texture1DArray.FirstArraySlice = viewDesc.texture.firstArraySlice;
                                result.Texture1DArray.MipSlice = viewDesc.texture.mipLevel;
                            }
                            else
                                result.Texture1D.MipSlice = viewDesc.texture.firstArraySlice;
                            break;
                        case TextureDescription::Dimension::Texture2D:
                        case TextureDescription::Dimension::TextureCube:
                            if (viewDesc.texture.firstArraySlice * arrayMultiplier > 1)
                            {
                                result.Texture2DArray.ArraySize = viewDesc.texture.arraySliceCount * arrayMultiplier;
                                result.Texture2DArray.FirstArraySlice = viewDesc.texture.firstArraySlice * arrayMultiplier;
                                result.Texture2DArray.MipSlice = viewDesc.texture.mipLevel;
                            }
                            else
                                result.Texture2D.MipSlice = viewDesc.texture.mipLevel;
                            break;
                        case TextureDescription::Dimension::Texture2DMS:
                            LOG_FATAL("Unsupported resource view type");
                            //ASSERT(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value)
                            break;
                        default:
                            LOG_FATAL("Unsupported resource view type");
                    }

                    return result;
                }

                template <typename DescType>
                DescType createDsvRtvDesc(const TextureDescription& textureDescription, const GpuResourceViewDescription& description)
                {
                    static_assert(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value);

                    DescType result = createDsvRtvUavDescCommon<DescType>(textureDescription, description);

                    if ((textureDescription.dimension == TextureDescription::Dimension::Texture2DMS) &&
                        (textureDescription.arraySize > 1))
                    {
                        result.Texture2DMSArray.ArraySize = description.texture.firstArraySlice;
                        result.Texture2DMSArray.FirstArraySlice = description.texture.arraySliceCount;
                    }

                    return result;
                }

                D3D12_DEPTH_STENCIL_VIEW_DESC createDsvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    ASSERT(resource);

                    switch (resource->GetResourceType())
                    {
                        case GpuResourceType::Texture: return createDsvRtvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(resource->GetTyped<Texture>()->GetDescription(), description);
                        default: ASSERT_MSG(false, "Unsupported resource type");
                    }

                    return D3D12_DEPTH_STENCIL_VIEW_DESC {};
                }

                D3D12_RENDER_TARGET_VIEW_DESC createRtvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    ASSERT(resource);

                    switch (resource->GetResourceType())
                    {
                        case GpuResourceType::Texture: return createDsvRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(resource->GetTyped<Texture>()->GetDescription(), description);
                        default: ASSERT_MSG(false, "Unsupported resource type");
                    }

                    return D3D12_RENDER_TARGET_VIEW_DESC {};
                }

                D3D12_UNORDERED_ACCESS_VIEW_DESC createUavDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    ASSERT(resource);

                    switch (resource->GetResourceType())
                    {
                        case GpuResourceType::Texture: return createDsvRtvUavDescCommon<D3D12_UNORDERED_ACCESS_VIEW_DESC>(resource->GetTyped<Texture>()->GetDescription(), description);
                        case GpuResourceType::Buffer: return createUavBufferDesc(resource->GetTyped<Buffer>()->GetDescription(), description);
                        default: ASSERT_MSG(false, "Unsupported resource type");
                    }

                    return D3D12_UNORDERED_ACCESS_VIEW_DESC {};
                }

                std::shared_ptr<DescriptorHeap> createDescpriptiorHeap(const DescriptorHeap::DescriptorHeapDesc& desc)
                {
                    const auto& heap = std::make_shared<DescriptorHeap>();
                    heap->Init(desc);

                    return heap;
                }
            }

            void DescriptorManager::Init()
            {
                ASSERT(!isInited_)

                {
                    DescriptorHeap::DescriptorHeapDesc desription;

                    desription.numDescriptors_ = 1000;
                    desription.name = "CpuCvbUavSrv";
                    desription.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                    desription.flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                    cbvUavSrvDescriptorHeap_ = createDescpriptiorHeap(desription);
                }

                {
                    DescriptorHeap::DescriptorHeapDesc desription;

                    desription.numDescriptors_ = 1000;
                    desription.name = "CpuRtv";
                    desription.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                    desription.flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                    rtvDescriptorHeap_ = createDescpriptiorHeap(desription);
                }
                /*
                const auto& device = DeviceContext::GetDevice();
                for (size_t index = 0; index < size_t(TextureDescription::Dimension::Count); index++)
                {
                    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                    rtvDesc.ViewDimension = getViewDimension<D3D12_RTV_DIMENSION>(GpuResourceDimension(index), false);
                    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

                    auto& descriptor = nullRtvDescriptors_[index];
                    rtvDescriptorHeap_->Allocate(descriptor);
                    device->CreateRenderTargetView(nullptr, &rtvDesc, descriptor.GetCPUHandle());
                }*/

                isInited_ = true;
            }

            void DescriptorManager::Terminate()
            {
                ASSERT(isInited_);

                std::destroy(std::begin(nullRtvDescriptors_), std::end(nullRtvDescriptors_));
                cbvUavSrvDescriptorHeap_ = nullptr;
                rtvDescriptorHeap_ = nullptr;

                isInited_ = false;
            }

            void DescriptorManager::Allocate(GpuResourceView& resourceView)
            {
                ASSERT(isInited_);

                const auto& resourceSharedPtr = resourceView.GetGpuResource().lock();
                ASSERT(resourceSharedPtr);

                const auto resourcePrivateImpl = resourceSharedPtr->GetPrivateImpl<ResourceImpl>();
                ASSERT(resourcePrivateImpl);

                const auto& resourceD3dObject = resourcePrivateImpl->GetD3DObject();
                ASSERT(resourceD3dObject);

                auto descriptor = std::make_unique<DescriptorHeap::Descriptor>();

                switch (resourceView.GetViewType())
                {
                    case GpuResourceView::ViewType::RenderTargetView:
                    {
                        rtvDescriptorHeap_->Allocate(*descriptor);
                        const auto& desc = createRtvDesc(resourceSharedPtr, resourceView.GetDescription());
                        DeviceContext::GetDevice()->CreateRenderTargetView(resourceD3dObject.get(), &desc, descriptor->GetCPUHandle());
                    }
                    break;
                    case GpuResourceView::ViewType::UnorderedAccessView:
                    {
                        cbvUavSrvDescriptorHeap_->Allocate(*descriptor);
                        const auto& desc = createUavDesc(resourceSharedPtr, resourceView.GetDescription());
                        DeviceContext::GetDevice()->CreateUnorderedAccessView(resourceD3dObject.get(), nullptr, &desc, descriptor->GetCPUHandle());
                    }
                    break;
                    default:
                        LOG_FATAL("Unsupported resource view type");
                }

                resourceView.SetPrivateImpl(descriptor.release());
            }
        }
    }
}