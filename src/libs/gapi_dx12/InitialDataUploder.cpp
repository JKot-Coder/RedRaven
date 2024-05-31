#include "InitialDataUploder.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

#include "gapi/GpuResource.hpp"

#include "common/threading/Mutex.hpp"

namespace RR::GAPI::DX12
{
    InitialDataUploder::InitialDataUploder()
    {
        fence_ = std::make_unique<FenceImpl>();
        commandList_ = std::make_unique<CommandListImpl>(CommandListType::Copy);
        commandQueue_ = std::make_unique<CommandQueueImpl>(CommandQueueType::Copy);
    }

    void InitialDataUploder::Init()
    {
        Threading::ReadWriteGuard lock(spinlock_);

        ASSERT(!inited_);

        fence_->Init("Initial uploads");
        commandList_->Init("Initial uploads");
        commandQueue_->Init("Initial uploads");

        inited_ = true;
    }

    void InitialDataUploder::Terminate()
    {
        Threading::ReadWriteGuard lock(spinlock_);

        ASSERT(inited_);

        fence_ = nullptr;
        commandList_ = nullptr;
        commandQueue_ = nullptr;

        inited_ = false;

        ASSERT(pendingDefferedUploads == 0);
    }

    void InitialDataUploder::DefferedUpload(const ResourceImpl& resourceImpl, const IDataBuffer::SharedPtr& initialData)
    {
        ASSERT(inited_);

        const auto& device = DeviceContext::GetDevice();

        const auto d3dResourceDesc = resourceImpl.GetD3DObject()->GetDesc();
        const uint32_t numSubresources = D3DUtils::GetSubresourcesCount(d3dResourceDesc);

        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(numSubresources);
        std::vector<UINT> numRowsVector(numSubresources);
        std::vector<UINT64> rowSizeInBytesVector(numSubresources);

        UINT64 totalSize;
        device->GetCopyableFootprints(&d3dResourceDesc, 0, numSubresources, 0, layouts.data(), numRowsVector.data(), rowSizeInBytesVector.data(), &totalSize);

        const auto& intermediateResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        const auto defaultState = D3D12_RESOURCE_STATE_GENERIC_READ;

        ComSharedPtr<ID3D12Resource> d3dresource;
        D3D12MA::Allocation* allocation;
        HRESULT hr = DeviceContext::GetAllocator()->CreateResource(
            &allocationDesc,
            &intermediateResourceDesc,
            defaultState,
            NULL,
            &allocation,
            IID_PPV_ARGS(d3dresource.put()));

        ASSERT(SUCCEEDED(hr));

        ResourceImpl intermediateResource;
        intermediateResource.Init(d3dresource, allocation, "(initial data)");

        auto dataPointer = intermediateResource.Map();
        // TODO TON OF ASSERTS AND LOGICK HERE
        memcpy(dataPointer, initialData->Data(), totalSize);

        intermediateResource.Unmap();

        const auto d3dCommandList = commandList_->GetD3DObject();
        const auto d3dDestinationResource = resourceImpl.GetD3DObject();
        const auto d3dIntermediateResource = intermediateResource.GetD3DObject();

        {
            Threading::ReadWriteGuard lock(spinlock_);

            pendingDefferedUploads++;

            if (d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                d3dCommandList->CopyBufferRegion(d3dDestinationResource.get(), 0, d3dIntermediateResource.get(), layouts[0].Offset, layouts[0].Footprint.Width);
            }
            else
            {
                for (uint32_t i = 0; i < numSubresources; ++i)
                {
                    CD3DX12_TEXTURE_COPY_LOCATION Dst(d3dDestinationResource.get(), i);
                    CD3DX12_TEXTURE_COPY_LOCATION Src(d3dIntermediateResource.get(), layouts[i]);
                    d3dCommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
                }
            }

            if (resourceImpl.GetDefaultResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
            {
                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3dDestinationResource.get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceImpl.GetDefaultResourceState());
                d3dCommandList->ResourceBarrier(1, &barrier);
            }
        }

        if (pendingDefferedUploads == UploadBatchSize)
            submit();
    }

    void InitialDataUploder::submit()
    {
        Threading::ReadWriteGuard lock(spinlock_);

        ASSERT(inited_);

        if (!pendingDefferedUploads)
            return;

        ASSERT(fence_);
        commandList_->Close();
        commandQueue_->Submit(*commandList_.get(), false);
        commandQueue_->Signal(*fence_.get());

        pendingDefferedUploads = 0;
    }

    void InitialDataUploder::FlushAndWaitFor(CommandQueueImpl& commandQueue)
    {
        ASSERT(inited_);

        submit();
        commandQueue.Wait(*fence_.get(), fence_->GetCpuValue());
    }
}