#pragma once

#include <memory>

namespace Rendering {

    class Mesh;

    class Primitives {
    public:
        static std::shared_ptr<Mesh> GetSphereMesh(unsigned int segments);
    };

}
