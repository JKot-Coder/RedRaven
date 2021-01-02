#pragma once

#include "rendering/Render.hpp"

typedef void* SDL_GLContext;

namespace OpenDemo
{
    namespace Rendering
    {
        struct BlendingDescription;
    }

    namespace Rendering
    {
        namespace OpenGL
        {
            typedef int GLint;
            typedef uint32_t GLuint;
            typedef uint32_t GLenum;

            class Render final : public Rendering::Render
            {
            public:
                Render();

                virtual void Init(const std::shared_ptr<Windowing::Window>& window) override;
                virtual void Terminate() override;

                virtual void SwapBuffers() const override;

                virtual void Clear(const Common::Vector4& color, float depth) const override;
                virtual void ClearColor(const Common::Vector4& color) const override;
                virtual void ClearDepthStencil(float depth) const override;

                virtual void Begin(const std::shared_ptr<RenderContext>& renderContext) override;
                virtual void DrawElement(const RenderElement& renderElement) const override;
                virtual void End() const override;

                virtual std::shared_ptr<Rendering::Texture2D> CreateTexture2D() const override;
                virtual std::shared_ptr<Rendering::Shader> CreateShader() const override;
                virtual std::shared_ptr<Rendering::Mesh> CreateMesh() const override;
                virtual std::shared_ptr<Rendering::RenderTargetContext> CreateRenderTargetContext() const override;

            private:
                void ApplyBlending(bool blending, const BlendingDescription& description) const;

                std::shared_ptr<Windowing::Window> _window;
                std::shared_ptr<RenderContext> _renderContext;
                SDL_GLContext _context;
            };
        }
    }
}