#include <SDL_video.h>

#include "dependencies/glad/glad.h"

#include "common/Utils.hpp"
#include "common/Exception.hpp"

#include "windowing/Window.hpp"

#include "render/Rendering.hpp"
#include "render/opengl/Shader.hpp"
#include "render/opengl/Mesh.hpp"

#include "render/opengl/Rendering.hpp"
#include "Rendering.hpp"


namespace Render {
    std::unique_ptr<Rendering> Render::Rendering::instance = std::unique_ptr<Rendering>(new OpenGL::Rendering());
}

namespace Render {
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
//            case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
//                return "invalid framebuffer operation";
        }
    };

    Rendering::Rendering() : context(nullptr) {

    }

    void Rendering::Init(const std::shared_ptr<Windowing::Window> &window) {
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
    }

    void Rendering::Terminate() {
        if (context){
            SDL_GL_DeleteContext(context);
        }
    }

    void Rendering::Update() const {
        auto windowSize = window->GetSize();
        glViewport(0, 0, windowSize.x, windowSize.y);
        glScissor(0, 0, windowSize.x, windowSize.y);
    }

    void Rendering::SwapBuffers() const {
        //TODO remove
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            fprintf(stderr, "GL_ERROR: %s\n", gluErrorString(error));
        }

        SDL_GL_SwapWindow(window->GetSDLWindow());
    }

    void Rendering::SetClearColor(const Common::vec4 &color) const {
        glClearColor(color.x, color.y, color.z, color.w);
        glClearDepth(1.0f); //Todo: Remove
    }

    void Rendering::Clear(bool color, bool depth) const {
        uint32_t mask = (color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0);
        if (mask) glClear(mask);
    }

    std::shared_ptr<Render::Shader> Rendering::CreateShader() const {
        return std::shared_ptr<OpenGL::Shader>(new OpenGL::Shader());
    }

    std::shared_ptr<Render::Mesh> Rendering::CreateMesh() const {
        return std::shared_ptr<OpenGL::Mesh>(new OpenGL::Mesh());
    }

}
}