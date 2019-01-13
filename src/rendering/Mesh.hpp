#pragma once

#include "common/VecMath.h"

namespace Rendering
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

        virtual void Init(const Vertex *vertices, int vCount) = 0;
        virtual void Init(const Vertex *vertices, int vCount, const int32_t *indexes, int iCount) = 0;

        virtual void Init(const std::vector<Vertex> &vertices) = 0;
        virtual void Init(const std::vector<Vertex> &vertices, const std::vector<int32_t> &indexes) = 0;

        virtual void Bind() const = 0;
        virtual void Draw() const = 0; //TODO: remove
    };

}