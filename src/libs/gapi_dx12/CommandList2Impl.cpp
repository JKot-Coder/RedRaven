#include "CommandList2Impl.hpp"

#include "gapi_dx12/DeviceContext.hpp"

#include "gapi/CommandList2.hpp"

#include "gapi/commands/SetRenderPass.hpp"


#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::DX12
{
    ComSharedPtr<ID3D12CommandAllocator> CommandList2Impl::CommandAllocatorsPool::createAllocator() const
    {
        ComSharedPtr<ID3D12CommandAllocator> allocator;

        D3DCall(DeviceContext::GetDevice()->CreateCommandAllocator(type_, IID_PPV_ARGS(allocator.put())));
      //  D3DUtils::SetAPIName(allocator.get(), name_, allocators_.size() + 1);

        return allocator;
    }

    CommandList2Impl::CommandAllocatorsPool::~CommandAllocatorsPool()
    {
        while (!allocators_.empty())
        {
            auto& allocator = allocators_.front();
            //ResourceReleaseContext::DeferredD3DResourceRelease(allocator.first);

            allocators_.pop();
        }
    }

    void CommandList2Impl::CommandAllocatorsPool::Init(
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

    ComSharedPtr<ID3D12CommandAllocator> CommandList2Impl::CommandAllocatorsPool::GetNextAllocator()
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

    void CommandList2Impl::Init(const CommandList2& commandList)
    {
        // TODO proper command list type
        allocatorsPool.Init(D3D12_COMMAND_LIST_TYPE_DIRECT, commandList.GetName());
        UNUSED(commandList);
    }

    void CommandList2Impl::Compile(CommandList2& commandList)
    {

        //ASSERT(commandList.size() != 0);

        for (const auto* command : commandList)
        {
            switch (command->type)
            {
            case Command::Type::SetRenderPass:
              //  compileCommand(static_cast<const Commands::SetRenderPass&>(*command), ctx);
                break;

            default:
                ASSERT_MSG(false, "Unsupported command type");
                break;
            }
        }

        commandList.clear();

       // commandBuffer = commandEncoder.finish();
        //commandEncoder.release();
    }
}