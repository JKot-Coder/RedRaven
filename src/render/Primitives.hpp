#pragma once

#include "memory"

namespace Render {

    class Mesh;

    class Primitives {
    public:
        static std::shared_ptr<Mesh> GetSphereMesh(uint32_t segments);
    };

}
