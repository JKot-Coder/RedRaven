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

        // TODO to UTILS?
        constexpr uint32_t planeSlices = 1;
        const uint32_t numFaces = (d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 6u : 1u);
        const uint32_t numSubresource = planeSlices * numFaces * d3dResourceDesc.DepthOrArraySize * d3dResourceDesc.MipLevels;

        UINT64 intermediateSize;
        device->GetCopyableFootprints(&d3dResourceDesc, 0, numSubresource, 0, nullptr, nullptr, nullptr, &intermediateSize);

        const auto& intermediateResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(intermediateSize);

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

        ResourceImpl intermediateResource;
        intermediateResource.Init(d3dresource, allocation, "(initial data)");

        auto dataPointer = intermediateResource.Map();
        // TODO TON OF ASSERTS AND LOGICK HERE
        memcpy(dataPointer, initialData->Data(), intermediateSize);

        intermediateResource.Unmap();

        {
            Threading::ReadWriteGuard lock(spinlock_);

            pendingDefferedUploads++;

            commandList_->CopyGpuResource(intermediateResource, resourceImpl);
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

        commandList_->Close();
        commandQueue_->Submit(*commandList_.get(), false);
        fence_->Signal(*commandQueue_.get());

        pendingDefferedUploads = 0;
    }

    void InitialDataUploder::FlushAndWaitFor(CommandQueueImpl& commandQueue)
    {
        ASSERT(inited_);

        submit();
        commandQueue.Wait(*fence_.get(), fence_->GetCpuValue());
    }
}