#pragma once

#include <memory>
#include <common/Stream.hpp>

#include "rendering/Mesh.hpp"

namespace ResourceManager {

    class ResourcesLoaders {
    public:
        static std::vector<std::shared_ptr<Rendering::Mesh>> LoadScene(const std::shared_ptr<Common::Stream> &stream);
    };

}