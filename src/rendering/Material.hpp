#pragma once

#include "memory"

#include "common/VecMath.h"

namespace Rendering {

    class CommonTexture;

    struct Material {
        Common::vec3 albedo;
        float roughness;

        std::shared_ptr<CommonTexture> albedoMap;
        std::shared_ptr<CommonTexture> normalMap;
        std::shared_ptr<CommonTexture> metallicMap;
        std::shared_ptr<CommonTexture> roughnessMap;
    };

}