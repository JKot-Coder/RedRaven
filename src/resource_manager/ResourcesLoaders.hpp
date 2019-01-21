#pragma once

#include <memory>
#include <string>

#include "rendering/Mesh.hpp"

namespace Common {
    class Stream;
}

namespace ResourceManager {

    class ResourcesLoaders {
    public:
        static const std::vector<Rendering::RenderElement> LoadScene(const std::string &fileName);
        static const std::shared_ptr<Rendering::CommonTexture> LoadTexture(const std::shared_ptr<Common::Stream> &stream);
    };

}