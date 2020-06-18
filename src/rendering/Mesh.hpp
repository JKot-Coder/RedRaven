#pragma once

#include <vector>

#include "common/VecMath.h"

namespace OpenDemo
{
    namespace Rendering
    {
#pragma pack(push, 1)
        struct Vertex
        {
            Common::vec3 position;
            Common::vec2 texCoord;
            Common::vec3 normal;
            Common::vec3 tangent;
            Common::vec3 binormal;
            Common::vec4 color;
        };
#pragma pack(pop)

        class Mesh
        {
        public:
            virtual ~Mesh() {};

            virtual void Init(const Vertex* vertices, int32_t vCount) = 0;
            virtual void Init(const Vertex* vertices, int32_t vCount, const int32_t* indexes, int32_t iCount) = 0;

            virtual void Init(const std::vector<Vertex>& vertices) = 0;
            virtual void Init(const std::vector<Vertex>& vertices, const std::vector<int32_t>& indexes) = 0;

            virtual void Bind() const = 0;
            virtual void Draw() const = 0; //TODO: remove
        };

    }
}