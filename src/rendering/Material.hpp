#pragma once

#include "common/VecMath.h"

namespace Rendering {

    struct Material {
        Common::vec3 albedo;
        float roughness;
    };

}