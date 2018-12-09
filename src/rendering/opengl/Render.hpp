#pragma once

#include "rendering/Render.hpp"

typedef void *SDL_GLContext;

namespace Rendering {
namespace OpenGL {

    class Render : public Rendering::Render {
    public:
        Render();

        virtual void Init(const std::shared_ptr<Windowing::Window>& window) override;
        virtual void Terminate() override;

        virtual void Update() const override;
        virtual void SwapBuffers() const override;

        virtual void SetClearColor(const Common::vec4 &color) const override;
        virtual void Clear(bool color, bool depth) const override;

        virtual std::shared_ptr<Rendering::Shader> CreateShader() const override;
        virtual std::shared_ptr<Rendering::Mesh> CreateMesh() const override;
    private:
        SDL_GLContext context;
    };

}
}