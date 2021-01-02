#pragma once

#include "rendering/RenderPasses.hpp"
#include "windowing/Windowing.hpp"

namespace OpenDemo
{
    namespace Rendering
    {
        class RenderPipeline final : Windowing::IListener
        {
        public:
            RenderPipeline(const std::shared_ptr<Windowing::Window>& window);
            ~RenderPipeline();

            void Init();
            void Collect(const std::shared_ptr<SceneGraph>& sceneGraph);
            void Draw();

        private:
            std::shared_ptr<Windowing::Window> _window;
            std::shared_ptr<RenderTargetContext> _hdrRenderTargetContext;

            std::tuple<
                std::unique_ptr<RenderPassOpaque>,
                std::unique_ptr<RenderPassPostProcess>>
                _renderPasses;

            template <typename PassType, typename... Args>
            void initPass(Args&&... args)
            {
                auto& passPtr = std::get<std::unique_ptr<PassType>>(_renderPasses);
                passPtr = std::make_unique<PassType>(std::forward<Args>(args)...);
            }

            template <typename PassType>
            inline PassType* getPass() const
            {
                return std::get<std::unique_ptr<PassType>>(_renderPasses).get();
            }

            virtual void OnWindowResize(const Windowing::Window& window) override;
        };
    }
}