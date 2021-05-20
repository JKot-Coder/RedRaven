#include "ResourceCreator.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DescriptorAllocator.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceViewsImpl.hpp"
#include "gapi_dx12/SwapChainImpl.hpp"

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

            void ResourceCreator::InitSwapChain(SwapChain& resource)
            {
                auto impl = std::make_unique<SwapChainImpl>();
                impl->Init(DeviceContext::GetDevice(), DeviceContext::GetDxgiFactory(), DeviceContext::GetGraphicsCommandQueue()->GetD3DObject(), resource.GetDescription(), resource.GetName());

                resource.SetPrivateImpl(impl.release());
            }

            void ResourceCreator::InitFence(Fence& resource)
            {
                auto impl = std::make_unique<FenceImpl>();
                impl->Init(resource.GetName());

                resource.SetPrivateImpl(impl.release());
            }

            void ResourceCreator::InitCommandQueue(CommandQueue& resource)
            {
                std::unique_ptr<CommandQueueImpl> impl;

                if (resource.GetCommandQueueType() == CommandQueueType::Graphics)
                {
                    static bool alreadyInited = false;
                    ASSERT(!alreadyInited); // Only one graphics command queue are alloved.
                    alreadyInited = true;

                    // Graphics command queue already initialized internally in device,
                    // so make copy to prevent d3d object leaking.
                    impl.reset(new CommandQueueImpl(*DeviceContext::GetGraphicsCommandQueue()));
                }
                else
                {
                    impl.reset(new CommandQueueImpl(resource.GetCommandQueueType()));
                    impl->Init(resource.GetName());
                }

                resource.SetPrivateImpl(impl.release());
            }

            void ResourceCreator::InitCommandList(CommandList& resource)
            {
                auto impl = std::make_unique<CommandListImpl>(resource.GetCommandListType());
                impl->Init(resource.GetName());

                resource.SetPrivateImpl(static_cast<ICommandList*>(impl.release()));
            }

            void ResourceCreator::InitGpuResourceView(GpuResourceView& object)
            {
                DescriptorAllocator::Instance().Allocate(object);
            }
        }
    }
}