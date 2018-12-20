#pragma once

#include <cstdint>

#include "rendering/Render.hpp"

typedef void *SDL_GLContext;

namespace Rendering {
namespace OpenGL {

    typedef uint32_t GLuint;
    typedef uint32_t GLenum;

    class Render final: public Rendering::Render {
    public:
        Render();

        virtual void Init(const std::shared_ptr<Windowing::Window>& window) override;
        virtual void Terminate() override;

        virtual void Update() const override;
        virtual void SwapBuffers() const override;

        virtual void Clear(const Common::vec4 &color, float depth) const override;
        virtual void ClearColor(const Common::vec4 &color) const override;
        virtual void ClearDepthStencil(float depth) const override;

        virtual void DrawElement(const RenderContext& renderContext, const RenderElement& renderElement) const override;

        virtual std::shared_ptr<Rendering::Shader> CreateShader() const override;
        virtual std::shared_ptr<Rendering::Mesh> CreateMesh() const override;
    private:
        SDL_GLContext context;
    };

}
}