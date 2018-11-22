#pragma once

#include "common/VecMath.h"

namespace Render
{
    class Mesh {
    public:
        virtual ~Mesh() {};
    };

    struct Vertex {
        Common::vec3 position;   // xyz  - position,
        Common::vec3 normal;     // xyz  - vertex normal
        Common::vec2 texCoord;   // xy   - texture coordinates
        Common::vec4 color;      // for non-textured geometry
    };
}