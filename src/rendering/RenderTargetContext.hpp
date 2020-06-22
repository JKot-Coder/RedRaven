#pragma once

#include <memory>

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
                ASSERT(renderTargetDescription.isDepthTarget)

                if (_width == -1 && _height == -1)
                {
                    _width = renderTargetDescription.texture->GetWidth();
                    _height = renderTargetDescription.texture->GetHeight();
                }

                ASSERT(renderTargetDescription.texture->GetHeight() == _height)
                ASSERT(renderTargetDescription.texture->GetWidth() == _width)

                _depthStencil = renderTargetDescription;
            }

            virtual inline void SetColorTarget(RenderTargetIndex index, const RenderTarget::RenderTargetDescription& renderTargetDescription)
            {
                if (_width == -1 && _height == -1)
                {
                    _width = renderTargetDescription.texture->GetWidth();
                    _height = renderTargetDescription.texture->GetHeight();
                }

                ASSERT(renderTargetDescription.texture->GetHeight() == _height)
                ASSERT(renderTargetDescription.texture->GetWidth() == _width)

                _colorTargets[index] = renderTargetDescription;
            }

            inline int GetWidth() const { return this->_width; }
            inline int GetHeight() const { return this->_height; }

            virtual inline void Resize(int width_, int height_)
            {
                //TODO asserts and checks for updated value
                _width = width_;
                _height = height_;

                if (_depthStencil.texture != nullptr)
                {
                    _depthStencil.texture->Resize(_width, _height);
                }

                for (auto targetDesription : _colorTargets)
                {
                    if (targetDesription.texture)
                    {
                        targetDesription.texture->Resize(_width, _height);
                    }
                }
            }

            virtual void Bind() = 0;

        private:
            int _width = -1, _height = -1;
            RenderTarget::RenderTargetDescription _depthStencil;
            RenderTarget::RenderTargetDescription _colorTargets[RenderTargetIndex::INDEX_MAX];
        };

    }
}