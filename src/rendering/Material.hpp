#pragma once

#include <memory>

namespace Rendering {

	class CommonTexture;

    struct Material {
		std::shared_ptr<Rendering::CommonTexture> albedoMap;
		std::shared_ptr<Rendering::CommonTexture> normalMap;
		std::shared_ptr<Rendering::CommonTexture> metallicMap;
		std::shared_ptr<Rendering::CommonTexture> roughnessMap;
    };

}