#pragma once

#include <memory>

#include "common/Common.hpp"

#include "rendering/RenderTarget.hpp"

namespace OpenDemo
{
    namespace Rendering
    {

        enum RenderTargetIndex
        {
            INDEX_0,
            INDEX_1,
            INDEX_2,
            INDEX_MAX
        };

        class RenderTargetContext
        {
        public:
            virtual inline void SetDepthStencilTarget(const RenderTarget::RenderTargetDescription& renderTargetDescription)
            {
                ASSERT(renderTargetDescription.isDepthTarget);

                if (width == -1 && height == -1)
                {
                    width = renderTargetDescription.texture->GetWidth();
                    height = renderTargetDescription.texture->GetHeight();
                }

                ASSERT(renderTargetDescription.texture->GetHeight() == height);
                ASSERT(renderTargetDescription.texture->GetWidth() == width);

                depthStencil = renderTargetDescription;
            }

            virtual inline void SetColorTarget(RenderTargetIndex index, const RenderTarget::RenderTargetDescription& renderTargetDescription)
            {
                if (width == -1 && height == -1)
                {
                    width = renderTargetDescription.texture->GetWidth();
                    height = renderTargetDescription.texture->GetHeight();
                }

                ASSERT(renderTargetDescription.texture->GetHeight() == height);
                ASSERT(renderTargetDescription.texture->GetWidth() == width);

                colorTargets[index] = renderTargetDescription;
            }

            inline int GetWidth() const { return this->width; }
            inline int GetHeight() const { return this->height; }

            virtual inline void Resize(int width_, int height_)
            {
                //TODO asserts and checks for updated value
                width = width_;
                height = height_;

                if (depthStencil.texture != nullptr)
                {
                    depthStencil.texture->Resize(width, height);
                }

                for (auto targetDesription : colorTargets)
                {
                    if (targetDesription.texture)
                    {
                        targetDesription.texture->Resize(width, height);
                    }
                }
            }

            virtual void Bind() = 0;

        private:
            int width = -1, height = -1;
            RenderTarget::RenderTargetDescription depthStencil;
            RenderTarget::RenderTargetDescription colorTargets[RenderTargetIndex::INDEX_MAX];
        };

    }
}