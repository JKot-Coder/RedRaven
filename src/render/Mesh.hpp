#pragma once

#include "common/VecMath.h"

namespace Render
{

    struct Vertex {
        Common::vec3 position;   // xyz  - position,
        Common::vec3 normal;     // xyz  - vertex normal
        Common::vec2 texCoord;   // xy   - texture coordinates
        Common::vec4 color;      // for non-textured geometry
    };

    class Mesh {
    public:
        virtual ~Mesh() {};

        virtual void Init(Vertex *vertices, int vCount) = 0;

        virtual void Bind() const = 0;
        virtual void Draw() const = 0; //TODO: remove
    };

}