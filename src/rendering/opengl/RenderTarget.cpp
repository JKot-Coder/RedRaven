#include "dependencies/glad/glad.h"

#include "RenderTarget.hpp"

namespace Rendering {
namespace OpenGL {

    RenderTarget::RenderTarget()
    {
        glGenFramebuffers(1, &id);
    }

    RenderTarget::~RenderTarget() {
        glDeleteFramebuffers(1, &id);
    }

    void RenderTarget::Init(const RenderTargetDescription& description) {
        Rendering::Texture2D::Description textureDescription;

        textureDescription.pixelFormat = description.pixelFormat;
        textureDescription.width = description.width;
        textureDescription.height = description.height;

        texture.Init(textureDescription, nullptr);
    }

//    void RenderTarget::Init() {
//        glBindFramebuffer(GL_FRAMEBUFFER, &id);
//
//        if Channel = rcDepth then
//        gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TargetID, TextureID, 0)
//        else
//        gl.FramebufferTexture2D(GL_FRAMEBUFFER, TGLConst(Ord(GL_COLOR_ATTACHMENT0) + Ord(Channel) - 1), TargetID, TextureID, 0);
//
//        Self.Texture[Channel] := Texture;
//        ChannelCount := 0;
//        for Channel := rcColor0 to High(Self.Texture) do
//            if Self.Texture[Channel] <> nil then
//        begin
//        ChannelList[ChannelCount] := TGLConst(Ord(GL_COLOR_ATTACHMENT0) + Ord(Channel) - 1);
//        Inc(ChannelCount);
//        end;
//        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    }

}
}