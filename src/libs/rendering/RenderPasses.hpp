#pragma once

#include "rendering/RenderContext.hpp"

#include <tuple>

namespace OpenDemo
{
    namespace Rendering
    {
        class Render;
        class SceneGraph;
        class Shader;
        class Texture2D;
        class RenderContext;
        class Mesh;

        enum class RenderPassType
        {
            // OPAQUE,
            POST_PROCESS
        };

        class RenderPass
        {
        public:
            virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) = 0;
            virtual void Draw() = 0;
        };

        class RenderPassOpaque final : public RenderPass
        {
        public:
            RenderPassOpaque(Rendering::Render& render, const std::shared_ptr<RenderTargetContext>& hdrRenderTargetContext);

            virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) override;
            virtual void Draw() override;

        private:
            Render* _render;
            std::shared_ptr<RenderTargetContext> _hdrRenderTargetContext;
            std::shared_ptr<RenderContext> _renderContext;
            std::shared_ptr<Shader> _pbrShader;
        };

        class RenderPassPostProcess final : public RenderPass
        {
        public:
            RenderPassPostProcess(Rendering::Render& render, const std::shared_ptr<Texture2D>& hdrTexture);

            virtual void Collect(const std::shared_ptr<SceneGraph>& sceneGraph) override;
            virtual void Draw() override;

        private:
            Render* _render;
            std::shared_ptr<Texture2D> _hdrTexture;
            std::shared_ptr<RenderContext> _renderContext;
            std::shared_ptr<Shader> _postProcessShader;
            std::shared_ptr<Mesh> _fullScreenQuad;
        };
    }
}