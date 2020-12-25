#include "ResourceViews.hpp"

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        RenderTargetView::SharedPtr RenderTargetView::Create(const Texture::SharedPtr& texture, const ResourceViewDescription& desc, const U8String& name)
        {
            ASSERT(texture);
            ASSERT(desc.texture.mipLevel + desc.texture.mipsCount <= texture->GetDescription().mipLevels)
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySlicesCount <= texture->GetDescription().arraySize)
            ASSERT(IsSet(texture->GetBindFlags(), Texture::BindFlags::RenderTarget))

            return RenderTargetView::SharedPtr(new RenderTargetView(texture, desc, name));
        }
    }
}