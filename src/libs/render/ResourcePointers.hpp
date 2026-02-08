#pragma once

#include "gapi/ForwardDeclarations.hpp"

namespace RR::Render
{
    template <typename T>
    struct ResourceDeleter {
        void operator()(T* resource) {
            UNUSED(resource);
          //  ResourceDeletionQueue::Instance().PushDelete(resource);
        }
    };


    template <typename T>
    using ResourceUniquePtr = eastl::unique_ptr<T, ResourceDeleter<T>>;

    using TextureUniquePtr = ResourceUniquePtr<GAPI::Texture>;
    using CommandQueueUniquePtr = ResourceUniquePtr<GAPI::CommandQueue>;
    using ShaderUniquePtr = ResourceUniquePtr<GAPI::Shader>;
    using BufferUniquePtr = ResourceUniquePtr<GAPI::Buffer>;
    using RenderTargetViewUniquePtr = ResourceUniquePtr<GAPI::RenderTargetView>;
    using DepthStencilViewUniquePtr = ResourceUniquePtr<GAPI::DepthStencilView>;
    using ShaderResourceViewUniquePtr = ResourceUniquePtr<GAPI::ShaderResourceView>;
    using UnorderedAccessViewUniquePtr = ResourceUniquePtr<GAPI::UnorderedAccessView>;
    using SwapChainUniquePtr = ResourceUniquePtr<GAPI::SwapChain>;
    using GraphicPipelineStateUniquePtr = ResourceUniquePtr<GAPI::GraphicPipelineState>;
    using BindingGroupUniquePtr = ResourceUniquePtr<GAPI::BindingGroup>;
}