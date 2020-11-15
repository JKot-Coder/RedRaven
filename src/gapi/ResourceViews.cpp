#include "ResourceViews.hpp"

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace
        {
            template <ResourceView::ViewType type>
            Resource::BindFlags GetRequiredBindFlags();

            template <>
            Resource::BindFlags GetRequiredBindFlags<ResourceView::ViewType::RenderTargetView>() { return Texture::BindFlags::RenderTarget; }

            template <>
            Resource::BindFlags GetRequiredBindFlags<ResourceView::ViewType::DepthStencilView>() { return Texture::BindFlags::DepthStencil; }

            template <>
            Resource::BindFlags GetRequiredBindFlags<ResourceView::ViewType::ShaderResourceView>() { return Texture::BindFlags::ShaderResource; }

            template <>
            Resource::BindFlags GetRequiredBindFlags<ResourceView::ViewType::UnorderedAccessView>() { return Texture::BindFlags::UnorderedAccess; }
        }

        template <ResourceView::ViewType type>
        std::shared_ptr<ResourceViewTemplate<type>> ResourceViewTemplate<type>::Create(const Texture::SharedPtr& texture, const ResourceView::Description& desc, const U8String& name)
        {
            ASSERT(texture);
            ASSERT(desc.texture.mipLevel + desc.texture.mipsCount <= texture->GetDescription().mipLevels)
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySlicesCount <= texture->GetDescription().arraySize)
            ASSERT(IsSet(texture->GetBindFlags(), GetRequiredBindFlags<type>()))

            return std::shared_ptr<ResourceViewTemplate<type>>(new ResourceViewTemplate<type>(texture, desc, name));
        }

        template class RenderTargetView::SharedPtr RenderTargetView::Create(const Texture::SharedPtr& texture, const ResourceView::Description& desc, const U8String& name);
    }
}