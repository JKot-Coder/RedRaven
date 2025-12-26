#include "CommandListImpl.hpp"

#include "gapi_dx12/DescriptorHeap.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/ResourceImpl.hpp"

#include "gapi/CommandList.hpp"

#include "gapi/commands/SetRenderPass.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::DX12
{
    namespace
    {
        struct CommandCompileContext
        {
            ID3D12GraphicsCommandList4* cmdList;
        };

        void compileCommand(const Commands::SetRenderPass& command, CommandCompileContext& ctx)
        {
            for (uint32_t i = 0; i < command.desc.colorAttachmentCount; i++)
            {
                if(command.desc.colorAttachments[i].loadOp == AttachmentLoadOp::Clear)
                {
                    const auto renderTargetView = command.desc.colorAttachments[i].renderTargetView;
                    const auto resourceImpl = renderTargetView->GetGpuResource().lock()->GetPrivateImpl<ResourceImpl>();
                    const auto descriptor = renderTargetView->GetPrivateImpl<DescriptorHeap::Descriptor>();

                    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    ctx.cmdList->ResourceBarrier(1, &barrier);

                    ctx.cmdList->ClearRenderTargetView(descriptor->GetCPUHandle(), &command.desc.colorAttachments[i].clearColor.x, 0, nullptr);

                    barrier = CD3DX12_RESOURCE_BARRIER::Transition(resourceImpl->GetD3DObject().get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                    ctx.cmdList->ResourceBarrier(1, &barrier);
                }
            }
        }
    }

    ComSharedPtr<ID3D12CommandAllocator> CommandListImpl::CommandAllocatorsPool::createAllocator() const
    {
        ComSharedPtr<ID3D12CommandAllocator> allocator;

        D3DCall(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())));
      //  D3DUtils::SetAPIName(allocator.get(), name_, allocators_.size() + 1);

        return allocator;
    }

    CommandListImpl::CommandAllocatorsPool::~CommandAllocatorsPool()
    {
        while (!allocators_.empty())
        {
            auto& allocator = allocators_.front();
            //ResourceReleaseContext::DeferredD3DResourceRelease(allocator.first);

            allocators_.pop();
        }
    }

    void CommandListImpl::CommandAllocatorsPool::Init(
        D3D12_COMMAND_LIST_TYPE type,
        const std::string& name)
    {
        UNUSED(name);
        type_ = type;
      //  name_ = name;
        fence_ = eastl::make_unique<FenceImpl>();
        fence_->Init(name);

        for (uint32_t index = 0; index < InitialAllocatorsCount; index++)
            allocators_.emplace(createAllocator(), 0);
    }

    ComSharedPtr<ID3D12CommandAllocator> CommandListImpl::CommandAllocatorsPool::GetNextAllocator()
    {
        auto allocator = allocators_.front();

        if (fence_->GetGpuValue() < allocator.second)
        {
            ASSERT_MSG(allocators_.size() < MaxAllocatorsCount,
                       "Too many allocators. Fence value: {}, Alocator fence value: {}, Allocator count: {}",
                       fence_->GetGpuValue(), allocator.second, allocators_.size());
            // The oldest allocator doesn't executed yet, create new one
            return allocators_.emplace(createAllocator(), fence_->GetCpuValue()).first;
        }

        // Reuse old one
        allocator.second = fence_->GetCpuValue();
        allocator.first->Reset();
        allocators_.pop();
        allocators_.push(allocator); // Place in the end

        return allocator.first;
    }

    void CommandListImpl::Init(const CommandList& commandList)
    {
        // TODO proper command list type
        const auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        allocatorsPool.Init(type, commandList.GetName());

        const auto allocator = allocatorsPool.GetNextAllocator();

        D3DCall(DeviceContext::GetDevice()->CreateCommandList(0, type, allocator.get(), nullptr, IID_PPV_ARGS(D3DCommandList.put())));
        D3DUtils::SetAPIName(D3DCommandList.get(), commandList.GetName());
    }

    void CommandListImpl::Compile(CommandList& commandList)
    {;
        D3DCommandList->Reset(allocatorsPool.GetNextAllocator().get(), nullptr);

        CommandCompileContext ctx{ D3DCommandList.get() };

        for (const auto* command : commandList)
        {
            switch (command->type)
            {
            case Command::Type::SetRenderPass:
                compileCommand(static_cast<const Commands::SetRenderPass&>(*command), ctx);
                break;

            default:
                ASSERT_MSG(false, "Unsupported command type");
                break;
            }
        }

        D3DCommandList->Close();
    }
}