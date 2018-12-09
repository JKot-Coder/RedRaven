#pragma once

#include <memory>

#include "rendering/RenderPasses.hpp"

namespace Common {
    struct vec4;
}

namespace Windowing {
    class Window;
}

namespace Rendering {

    class Shader;
    class Mesh;

    enum Attributes {
        POSITION,
        NORMAL,
        TEXCOORD,
        COLOR,
        MAX_ATTRIBUTES
    };

    class Render {
    public:
        inline static const std::unique_ptr<Render>& Instance() {
            return instance;
        }

        virtual ~Render() {}

        virtual void Init(const std::shared_ptr<Windowing::Window> &window) = 0;
        virtual void Terminate() = 0;

        virtual void Update() const = 0;
        virtual void SwapBuffers() const = 0;

        virtual void SetClearColor(const Common::vec4 &color) const = 0;
        virtual void Clear(bool color, bool depth) const = 0;

        virtual std::shared_ptr<Shader> CreateShader() const = 0;
        virtual std::shared_ptr<Mesh> CreateMesh() const = 0;

    protected:
        std::shared_ptr<Windowing::Window> window;

    private:
        static std::unique_ptr<Render> instance;
        static std::tuple<
            std::unique_ptr<RenderPassOpaque>
        > renderPasses;
    };

    inline static const std::unique_ptr<Render>& Instance() {
        return Render::Instance();
    }

}