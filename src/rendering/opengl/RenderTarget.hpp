#pragma once

#include "rendering/RenderTarget.hpp"
#include "rendering/opengl/Texture.hpp"

namespace Rendering {
namespace OpenGL {

    class RenderTarget final : public Rendering::RenderTargetTexture {
    public:
        RenderTarget();
        virtual ~RenderTarget() override;

        virtual void Init(const RenderTargetDescription& description) override;

        inline virtual void Bind(int sampler) override {
            texture.Bind(sampler);
        }
    private:
        GLuint id;
        Texture2D texture;
    };

}
}