#pragma once

#include "rendering/Material.hpp"

#include "common/Math.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        class Window;
    }

    namespace Rendering
    {

        class Texture2D;
        class Shader;
        class Mesh;
        class RenderCommandContext;
        class RenderTargetContext;

        enum PixelFormat : int
        {
            R8,
            RG8,
            RGB8,
            RGBA8,
            RGB565,
            RGB5_A1,
            RGBA32F,
            RGBA16F,
            D16,
            PIXEL_FORMAT_MAX
        };

        enum Attributes
        {
            POSITION,
            TEXCOORD,
            NORMAL,
            TANGENT,
            BINORMAL,
            COLOR,
            MAX_ATTRIBUTES
        };

        struct RenderElement
        {
        public:
            Matrix4 modelMatrix;
            Rendering::Material material;
            std::shared_ptr<Mesh> mesh;
        };

        class Render
        {
        public:
            inline static const std::unique_ptr<Render>& Instance()
            {
                return instance;
            }

            virtual ~Render() { }

            virtual void Init(const std::shared_ptr<Windowing::Window>& window) = 0;
            virtual void Terminate() = 0;

            virtual void SwapBuffers() const = 0;

            virtual void Clear(const Common::Vector4& color, float depth) const = 0;
            virtual void ClearColor(const Common::Vector4& color) const = 0;
            virtual void ClearDepthStencil(float depth) const = 0;

            virtual void Begin(const std::shared_ptr<RenderCommandContext>& RenderCommandContext) = 0;
            virtual void DrawElement(const RenderElement& renderElement) const = 0;
            virtual void End() const = 0;

            virtual std::shared_ptr<Texture2D> CreateTexture2D() const = 0;
            virtual std::shared_ptr<Shader> CreateShader() const = 0;
            virtual std::shared_ptr<Mesh> CreateMesh() const = 0;
            virtual std::shared_ptr<RenderTargetContext> CreateRenderTargetContext() const = 0;

        protected:
            static std::unique_ptr<Render> instance;
        };

        inline static const std::unique_ptr<Render>& Instance()
        {
            return Render::Instance();
        }
    }
}