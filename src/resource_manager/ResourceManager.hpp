#pragma once

#include "rendering/Texture.hpp"

#include <vector>

namespace OpenDemo
{
    namespace Rendering
    {
        class Shader;
        class Mesh;
        struct RenderElement;
    }

    namespace ResourceManager
    {
        class ResourceManager
        {
        public:
            inline static const std::unique_ptr<ResourceManager>& Instance()
            {
                return _instance;
            }

            const std::shared_ptr<Rendering::Shader> LoadShader(const U8String& filename);
            const std::vector<Rendering::RenderElement> LoadScene(const U8String& filename);
            const std::shared_ptr<Rendering::CommonTexture> LoadTexture(const U8String& filename);

        private:
            static std::unique_ptr<ResourceManager> _instance;
        };

        inline static const std::unique_ptr<ResourceManager>& Instance()
        {
            return ResourceManager::Instance();
        }
    }
}