#include <SDL_video.h>

#include "dependencies/glad/glad.h"

#include "common/Utils.hpp"
#include "common/Exception.hpp"

#include "windowing/Window.hpp"

#include "rendering/Camera.hpp"
#include "rendering/Render.hpp"
#include "rendering/RenderContext.hpp"

#include "rendering/opengl/Texture.hpp"
#include "rendering/opengl/Shader.hpp"
#include "rendering/opengl/Mesh.hpp"
#include "rendering/opengl/Render.hpp"
#include "rendering/opengl/RenderTargetContext.hpp"
#include "Render.hpp"


namespace Rendering {
    std::unique_ptr<Render> Rendering::Render::instance = std::unique_ptr<Render>(new OpenGL::Render());
}

namespace Rendering {
namespace OpenGL {

    const auto gluErrorString = [](GLenum errorCode)->const char *
    {
        switch(errorCode)
        {
            default:
                return "unknown error code";
            case GL_NO_ERROR:
                return "no error";
            case GL_INVALID_ENUM:
                return "invalid enumerant";
            case GL_INVALID_VALUE:
                return "invalid value";
            case GL_INVALID_OPERATION:
                return "invalid operation";
            case GL_STACK_OVERFLOW:
                return "stack overflow";
            case GL_STACK_UNDERFLOW:
                return "stack underflow";
//            case GL_TABLE_TOO_LARGE:
//                return "table too large";
            case GL_OUT_OF_MEMORY:
                return "out of memory";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "invalid framebuffer operation";
        }
    };

    GLuint GetOpenGLDepthTestFunction(DepthTestFunction depthTestFunction) {
        static const GLuint depthTestFunctions[DEPTH_TEST_FUNC_MAX] = {
            GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS
        };

        return depthTestFunctions[depthTestFunction];
    }

    Render::Render() : context(nullptr) {

    }

    void Render::Init(const std::shared_ptr<Windowing::Window> &window) {
        this->window = window;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

        context = SDL_GL_CreateContext(window->GetSDLWindow());

        if (!context)
            throw Common::Exception("Can't create OpenGl context with error: %s.", SDL_GetError());

        if (!gladLoadGL())
            throw Common::Exception("Can't initalize openGL.");

        LOG("OpenGL Version %d.%d loaded \n", GLVersion.major, GLVersion.minor);

        if (!GLAD_GL_VERSION_3_3)
            throw Common::Exception("OpenGL version is not supported.");

        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
       // glDisable(GL_CULL_FACE);
    }

    void Render::Terminate() {
        if (context){
            SDL_GL_DeleteContext(context);
        }
    }

    void Render::SwapBuffers() const {
        //TODO remove
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            fprintf(stderr, "GL_ERROR: %d : %s\n", error, gluErrorString(error));
        }

        SDL_GL_SwapWindow(window->GetSDLWindow());
    }

    void Render::Clear(const Common::vec4 &color, float depth) const {
        glClearColor(color.x, color.y, color.z, color.w);
        glClearDepth(depth);
        //glClearBuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Render::ClearColor(const Common::vec4 &color) const {
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Render::ClearDepthStencil(float depth) const {
        glClearDepth(depth);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Render::Begin(const std::shared_ptr<RenderContext> &renderContext) {
        this->renderContext = renderContext;

        const auto& camera = renderContext->GetCamera();
        const auto& shader = renderContext->GetShader();
        const auto& renderTarget = renderContext->GetRenderTarget();

        float rtWidth, rtHeight;
        if (renderTarget == nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            rtWidth = window->GetWidth();
            rtHeight = window->GetHeight();
        } else {
            renderTarget->Bind();
            rtWidth = renderTarget->GetWidth();
            rtHeight = renderTarget->GetHeight();
        }

        glViewport(0, 0, rtWidth, rtHeight);
        glScissor(0, 0, rtWidth, rtHeight);

        glDepthMask(renderContext->GetDepthWrite());

        const auto depthTestFunction = renderContext->GetDepthTestFunction();
        if (depthTestFunction == ALWAYS){
            glDisable(GL_DEPTH_TEST);
        } else{
            glEnable(GL_DEPTH_TEST);
        }

        glDepthFunc(GetOpenGLDepthTestFunction(depthTestFunction));

        ApplyBlending(renderContext->GetBlending(), renderContext->GetBlendingDescription());

        shader->Bind();
        shader->SetParam(Shader::UniformType::LIGHT_DIR, renderContext->GetLightDirection());

        if (camera != nullptr) {
            camera->SetAspect(rtWidth, rtHeight);
            shader->SetParam(Shader::UniformType::VIEW_PROJECTION_MATRIX, camera->GetViewProjectionMatrix());
            shader->SetParam(Shader::UniformType::CAMERA_POSITION, camera->GetTransform().Position);
        }
    }

    void Render::DrawElement(const RenderElement &renderElement) const {
        const auto& shader = renderContext->GetShader();

        shader->SetParam(Shader::UniformType::MODEL_MATRIX, renderElement.modelMatrix);
        shader->SetParam(Shader::UniformType::MATERIAL, vec4(renderElement.material.roughness,1,1,1));

        renderElement.mesh->Draw();
    }

    void Render::End() const {

    }

    std::shared_ptr<Rendering::Texture2D> Render::CreateTexture2D() const {
        return std::shared_ptr<OpenGL::Texture2D>(new OpenGL::Texture2D());
    }

    std::shared_ptr<Rendering::Shader> Render::CreateShader() const {
        return std::shared_ptr<OpenGL::Shader>(new OpenGL::Shader());
    }

    std::shared_ptr<Rendering::Mesh> Render::CreateMesh() const {
        return std::shared_ptr<OpenGL::Mesh>(new OpenGL::Mesh());
    }

    std::shared_ptr<Rendering::RenderTargetContext> Render::CreateRenderTargetContext() const {
        return std::shared_ptr<OpenGL::RenderTargetContext>(new OpenGL::RenderTargetContext);
    }

    void Render::ApplyBlending(bool blending, const BlendingDescription &description) const {
        if (!blending) {
            glDisable(GL_BLEND);
            return;
        }

        (void) description; //TODO: Replace to true realization of this method
        glEnable(GL_BLEND);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//TODO: find a better place for it
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    }

}
}