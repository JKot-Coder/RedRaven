#include "DescriptorManager.hpp"

#include "gapi/GpuResource.hpp"

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
                DescType getViewDimension(GpuResourceDimension dimension, bool isTextureArray);

                template <>
                D3D12_RTV_DIMENSION getViewDimension(GpuResourceDimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case GpuResourceDimension::Buffer: return D3D12_RTV_DIMENSION_BUFFER;
                        case GpuResourceDimension::Texture1D: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
                        case GpuResourceDimension::Texture2D: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
                        case GpuResourceDimension::Texture3D: ASSERT(isTextureArray == false); return D3D12_RTV_DIMENSION_TEXTURE3D;
                        case GpuResourceDimension::Texture2DMS: return (isTextureArray) ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                        case GpuResourceDimension::TextureCube: return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_RTV_DIMENSION_UNKNOWN;
                }

                template <>
                D3D12_DSV_DIMENSION getViewDimension(GpuResourceDimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case GpuResourceDimension::Texture1D: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE1DARRAY : D3D12_DSV_DIMENSION_TEXTURE1D;
                        case GpuResourceDimension::Texture2D: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
                        case GpuResourceDimension::Texture2DMS: return (isTextureArray) ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                        case GpuResourceDimension::TextureCube: return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_DSV_DIMENSION_UNKNOWN;
                }

                template <>
                D3D12_UAV_DIMENSION getViewDimension(GpuResourceDimension dimension, bool isTextureArray)
                {
                    switch (dimension)
                    {
                        case GpuResourceDimension::Buffer: return D3D12_UAV_DIMENSION_BUFFER;
                        case GpuResourceDimension::Texture1D: return (isTextureArray) ? D3D12_UAV_DIMENSION_TEXTURE1DARRAY : D3D12_UAV_DIMENSION_TEXTURE1D;
                        case GpuResourceDimension::Texture2D: return (isTextureArray) ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D;
                        case GpuResourceDimension::TextureCube: return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    }

                    ASSERT_MSG(false, "Wrong resource dimension");
                    return D3D12_UAV_DIMENSION_UNKNOWN;
                }

                template <typename DescType>
                void initDsvRtvUavBufferDesc(DescType& description, const GpuResourceDescription& gpuResDesc, const GpuResourceViewDescription& viewDesc);

                template <>
                void initDsvRtvUavBufferDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& description, const GpuResourceDescription& gpuResDesc, const GpuResourceViewDescription& viewDesc)
                {
                    LOG_FATAL("Unsupported resource view type");
                }

                template <>
                void initDsvRtvUavBufferDesc(D3D12_RENDER_TARGET_VIEW_DESC& description, const GpuResourceDescription& gpuResDesc, const GpuResourceViewDescription& viewDesc)
                {
                    LOG_FATAL("Unsupported resource view type");
                }

                template <>
                void initDsvRtvUavBufferDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& description, const GpuResourceDescription& gpuResDesc, const GpuResourceViewDescription& viewDesc)
                {
                    description.Buffer.StructureByteStride = gpuResDesc.GetStructSize();
                    description.Buffer.CounterOffsetInBytes = 0;

                    if (!gpuResDesc.IsTyped())
                    {
                        description.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
                    }
                    description.Format = D3DUtils::GetDxgiTypelessFormat(viewDesc.format);
                    description.Buffer.FirstElement = viewDesc.buffer.firstElement;
                    description.Buffer.NumElements = viewDesc.buffer.elementCount;
                }

                template <typename DescType>
                DescType createDsvRtvUavDescCommon(const GpuResourceDescription& gpuResDesc, const GpuResourceViewDescription& viewDesc)
                {
                    DescType result = {};

                    const uint32_t arraySize = (gpuResDesc.GetDimension() == GpuResourceDimension::Buffer) ? 1 : gpuResDesc.GetArraySize();
                    const uint32_t arrayMultiplier = (gpuResDesc.GetDimension() == GpuResourceDimension::TextureCube) ? 6 : 1;

                    ASSERT((gpuResDesc.GetDimension() == GpuResourceDimension::Buffer) ||
                           ((viewDesc.texture.firstArraySlice + viewDesc.texture.arraySliceCount) * arrayMultiplier <= arraySize));

                    result.ViewDimension = getViewDimension<decltype(result.ViewDimension)>(gpuResDesc.GetDimension(), arraySize > 1);
                    result.Format = D3DUtils::GetDxgiResourceFormat(gpuResDesc.GetFormat());

                    switch (gpuResDesc.GetDimension())
                    {
                        case GpuResourceDimension::Buffer:
                            initDsvRtvUavBufferDesc(result, gpuResDesc, viewDesc);
                            break;
                        case GpuResourceDimension::Texture1D:
                            if (viewDesc.texture.arraySliceCount > 1)
                            {
                                result.Texture1DArray.ArraySize = viewDesc.texture.arraySliceCount;
                                result.Texture1DArray.FirstArraySlice = viewDesc.texture.firstArraySlice;
                                result.Texture1DArray.MipSlice = viewDesc.texture.mipLevel;
                            }
                            else
                                result.Texture1D.MipSlice = viewDesc.texture.firstArraySlice;
                            break;
                        case GpuResourceDimension::Texture2D:
                        case GpuResourceDimension::TextureCube:
                            if (viewDesc.texture.firstArraySlice * arrayMultiplier > 1)
                            {
                                result.Texture2DArray.ArraySize = viewDesc.texture.arraySliceCount * arrayMultiplier;
                                result.Texture2DArray.FirstArraySlice = viewDesc.texture.firstArraySlice * arrayMultiplier;
                                result.Texture2DArray.MipSlice = viewDesc.texture.mipLevel;
                            }
                            else
                                result.Texture2D.MipSlice = viewDesc.texture.mipLevel;
                            break;
                        case GpuResourceDimension::Texture2DMS:
                            LOG_FATAL("Unsupported resource view type");
                            //ASSERT(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value)
                            break;
                        default:
                            LOG_FATAL("Unsupported resource view type");
                    }

                    return result;
                }

                template <typename DescType>
                DescType createDsvRtvDesc(const GpuResourceDescription& gpuResourceDescription, const GpuResourceViewDescription& description)
                {
                    static_assert(std::is_same<DescType, D3D12_DEPTH_STENCIL_VIEW_DESC>::value || std::is_same<DescType, D3D12_RENDER_TARGET_VIEW_DESC>::value);

                    DescType result = createDsvRtvUavDescCommon<DescType>(gpuResourceDescription, description);

                    if ((gpuResourceDescription.GetDimension() == GpuResourceDimension::Texture2DMS) &&
                        (gpuResourceDescription.GetArraySize() > 1))
                    {
                        result.Texture2DMSArray.ArraySize = description.texture.firstArraySlice;
                        result.Texture2DMSArray.FirstArraySlice = description.texture.arraySliceCount;
                    }

                    return result;
                }

                D3D12_DEPTH_STENCIL_VIEW_DESC createDsvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    return createDsvRtvDesc<D3D12_DEPTH_STENCIL_VIEW_DESC>(resource->GetDescription(), description);
                }

                D3D12_RENDER_TARGET_VIEW_DESC createRtvDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    return createDsvRtvDesc<D3D12_RENDER_TARGET_VIEW_DESC>(resource->GetDescription(), description);
                }

                D3D12_UNORDERED_ACCESS_VIEW_DESC createUavDesc(const GpuResource::SharedPtr& resource, const GpuResourceViewDescription& description)
                {
                    return createDsvRtvUavDescCommon<D3D12_UNORDERED_ACCESS_VIEW_DESC>(resource->GetDescription(), description);
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

                const auto& device = DeviceContext::GetDevice();
                for (size_t index = 0; index < size_t(GpuResourceDimension::Count); index++)
                {
                    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                    rtvDesc.ViewDimension = getViewDimension<D3D12_RTV_DIMENSION>(GpuResourceDimension(index), false);
                    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

                    auto& descriptor = nullRtvDescriptors_[index];
                    rtvDescriptorHeap_->Allocate(descriptor);
                    device->CreateRenderTargetView(nullptr, &rtvDesc, descriptor.GetCPUHandle());
                }

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