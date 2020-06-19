#pragma once

#include <memory>
#include <string>
#include <vector>

#include "rendering/Render.hpp"
#include "rendering/Texture.hpp"

namespace OpenDemo
{
    namespace Common
    {
        class Stream;
    }

    namespace ResourceManager
    {

        class ResourcesLoaders
        {
        public:
            static const std::vector<Rendering::RenderElement> LoadScene(const U8String& fileName);
            static const std::shared_ptr<Rendering::CommonTexture> LoadTexture(const std::shared_ptr<Common::Stream>& stream);
        };

    }
}