#include "dependencies/glad/glad.h"

#include "rendering/opengl/RenderTargetContext.hpp"
#include "RenderTargetContext.hpp"
#include "Texture.hpp"


namespace Rendering {
namespace OpenGL {

    RenderTargetContext::RenderTargetContext() {
        glGenFramebuffers(1, &id);
    }

    RenderTargetContext::~RenderTargetContext() {
        glDeleteFramebuffers(1, &id);
    }

    void RenderTargetContext::SetColorTarget(Rendering::RenderTargetIndex index,
                                             const RenderTarget::RenderTargetDescription &renderTargetDescription) {
        Rendering::RenderTargetContext::SetColorTarget(index, renderTargetDescription);

        Bind();

        auto const &texture = renderTargetDescription.texture;
        if (texture) {
            auto const &openGlTexture = std::dynamic_pointer_cast<Rendering::OpenGL::Texture2D, Rendering::CommonTexture>(texture);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, openGlTexture->GetNativeId(), 0);
        }
    }

    void RenderTargetContext::SetDepthStencilTarget(const RenderTarget::RenderTargetDescription &renderTargetDescription) {
        Rendering::RenderTargetContext::SetDepthStencilTarget(renderTargetDescription);
    }

    void RenderTargetContext::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers);
    }
}
}