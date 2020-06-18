#pragma once

#include "rendering/Render.hpp"
#include "rendering/Texture.hpp"

namespace OpenDemo
{
    namespace Rendering
    {

        class RenderTarget
        {
        public:
            struct RenderTargetDescription
            {
                bool isDepthTarget;
                std::shared_ptr<CommonTexture> texture;
            };

            //        virtual ~RenderTarget() {};

            //        inline void Init(const RenderTargetDescription& description) {
            //            //TODO validation depth format.
            //
            //            texture = description.texture;
            //            isDepthTarget = description.isDepthTarget;
            //        }
            //
            //        inline bool IsDepthTarget() const { return isDepthTarget; }
            //        inline std::shared_ptr<CommonTexture> GetTexture() const { return texture; }
            //        inline float GetWidth() const { return texture->GetWidth(); }
            //        inline float GetHeight() const { return texture->GetHeight(); }
            //
            //    private:
            //        bool isDepthTarget;
            //        std::shared_ptr<CommonTexture> texture;
        };

    }
}