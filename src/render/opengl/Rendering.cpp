#include <SDL_video.h>

#include "common/Exception.hpp"

#include "windowing/Window.hpp"

#include "render/Rendering.hpp"
#include "render/opengl/Rendering.hpp"
#include "Rendering.hpp"

namespace Render {
    std::unique_ptr<Rendering> Render::Rendering::instance = std::unique_ptr<Rendering>(new OpenGL::Rendering());
}

namespace Render {
namespace OpenGL {

    Rendering::Rendering() : context(nullptr) {

    }

    void Rendering::Init(const std::shared_ptr<Windowing::Window>& window) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

        context = SDL_GL_CreateContext(window.get()->GetSDLWindow());

        if (!context)
            Common::Exception("Can't create OpenGl context with error: %s.", SDL_GetError());

        if (!gladLoadGL())
            Common::Exception("Can't initalize openGL.");

        printf("OpenGL Version %d.%d loaded", GLVersion.major, GLVersion.minor);

        if (!GLAD_GL_VERSION_3_3) {
            Common::Exception("OpenGL version is not supported.");
        }
    }

    void Rendering::Terminate() {
        if (context){
            SDL_GL_DeleteContext(context);
        }
    }

}
}