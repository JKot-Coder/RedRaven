#pragma once

#include <memory>

namespace Common {
    struct vec4;
}

namespace Windowing {
    class Window;
}

namespace Rendering {

    class Texture2D;
    class Shader;
    class Mesh;
    struct RenderElement;
    class RenderContext;
    class RenderTargetContext;

    enum PixelFormat : int {
        RGBA8,
        RGB565,
        RGB5_A1,
        RGBA32F,
        RGBA16F,
        D16,
        PIXEL_FORMAT_MAX
    };

    enum DepthTestFunction : int {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS,
        DEPTH_TEST_FUNC_MAX
    };

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

        virtual void SwapBuffers() const = 0;

        virtual void Clear(const Common::vec4 &color, float depth) const = 0;
        virtual void ClearColor(const Common::vec4 &color) const = 0;
        virtual void ClearDepthStencil(float depth) const = 0;

        virtual void Begin(const std::shared_ptr<RenderContext> &renderContext) = 0;
        virtual void DrawElement(const RenderElement& renderElement) const = 0;
        virtual void End() const = 0;

        virtual std::shared_ptr<Texture2D> CreateTexture2D() const = 0;
        virtual std::shared_ptr<Shader> CreateShader() const = 0;
        virtual std::shared_ptr<Mesh> CreateMesh() const = 0;
        virtual std::shared_ptr<RenderTargetContext> CreateRenderTargetContext() const = 0;

    protected:
        static std::unique_ptr<Render> instance;
    };

    inline static const std::unique_ptr<Render>& Instance() {
        return Render::Instance();
    }

}