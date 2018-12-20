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

    enum PixelFormat {
        RGBA8,
        RGB565,
        RGB5_A1,
        RGBA32F,
        RGBA16F,
        D16,
        PIXEL_FORMAT_MAX
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

        virtual void Update() const = 0;
        virtual void SwapBuffers() const = 0;

        virtual void Clear(const Common::vec4 &color, float depth) const = 0;
        virtual void ClearColor(const Common::vec4 &color) const = 0;
        virtual void ClearDepthStencil(float depth) const = 0;

        void Collect(const std::shared_ptr<SceneGraph>& sceneGraph);
        void Draw();
        virtual void DrawElement(const RenderContext& renderContext, const RenderElement& renderElement) const = 0;

        virtual std::shared_ptr<Shader> CreateShader() const = 0;
        virtual std::shared_ptr<Mesh> CreateMesh() const = 0;

    protected:
        void Init();

        std::shared_ptr<Windowing::Window> window;
        std::shared_ptr<Shader> pbrShader;

    private:
        template<typename PassType, typename... Args>
        void initPass(Args&&... args)
        {
            auto& passPtr = std::get<std::unique_ptr<PassType>>(renderPasses);
            passPtr = std::make_unique<PassType>(std::forward<Args>(args)...);
        }

        template<typename PassType>
        inline PassType* getPass() const
        {
            return std::get<std::unique_ptr<PassType>>(renderPasses).get();
        }

    private:
        std::tuple<
            std::unique_ptr<RenderPassOpaque>
        > renderPasses;

        static std::unique_ptr<Render> instance;
    };

    inline static const std::unique_ptr<Render>& Instance() {
        return Render::Instance();
    }

}