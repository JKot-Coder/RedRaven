#pragma once

#include "rendering/RenderTargetContext.hpp"
#include "rendering/opengl/Render.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        namespace OpenGL
        {
            class RenderTargetContext final : public Rendering::RenderTargetContext
            {
            public:
                RenderTargetContext();
                ~RenderTargetContext();

                virtual void SetDepthStencilTarget(const RenderTarget::RenderTargetDescription& renderTargetDescription) override;
                virtual void SetColorTarget(RenderTargetIndex index, const RenderTarget::RenderTargetDescription& renderTargetDescription) override;

                virtual void Bind() override;

            private:
                GLuint _id;
            };
        }
    }
}