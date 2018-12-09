#include <vector>
#include <array>

#include "common/VecMath.h"

#include "rendering/Render.hpp"
#include "rendering/Mesh.hpp"

#include "rendering/Primitives.hpp"

using namespace Common;

namespace Rendering {

    std::shared_ptr<Mesh> Primitives::GetSphereMesh(uint32_t segments)
    {
        const auto& render = Rendering::Instance();
        const auto& mesh = render->CreateMesh();

        const uint32_t vertexCount = segments * (segments - 1) + 2;
        auto vertices = new Vertex[vertexCount];

        uint32_t index = 0;
        vertices[index].position = vec3(0.0f, 1.0f, 0.0f);
        vertices[index].normal   = vec3(0.0f, 1.0f, 0.0f);
        index++;

        for (uint32_t j = 0; j < segments - 1; ++j)
        {
            double const polar = M_PI * double(j+1) / double(segments);
            double const sp = sin(polar);
            double const cp = cos(polar);

            for (uint32_t i = 0; i < segments; ++i)
            {
                double const azimuth = 2.0 * M_PI * double(i) / double(segments);
                double const sa = sin(azimuth);
                double const ca = cos(azimuth);
                double const x = sp * ca;
                double const y = cp;
                double const z = sp * sa;


                vertices[index].position = vec3(x, y, z);
                vertices[index].normal   = vec3(x, y, z);
                index++;
            }
        }

        vertices[index].position = vec3(0.0f, -1.0f, 0.0f);
        vertices[index].normal   = vec3(0.0f, -1.0f, 0.0f);

        std::vector<Vertex> *triangles = new std::vector<Vertex>();

        for (uint32_t i = 0; i < segments; ++i)
        {
            uint32_t const a = i + 1;
            uint32_t const b = (i + 1) % segments + 1;

            triangles->emplace_back(vertices[0]);
            triangles->emplace_back(vertices[b]);
            triangles->emplace_back(vertices[a]);
        }

        for (uint32_t j = 0; j < segments - 2; ++j)
        {
            uint32_t aStart = j * segments + 1;
            uint32_t bStart = (j + 1) * segments + 1;
            for (uint32_t i = 0; i < segments; ++i)
            {
                const uint32_t a = aStart + i;
                const uint32_t a1 = aStart + (i + 1) % segments;
                const uint32_t b = bStart + i;
                const uint32_t b1 = bStart + (i + 1) % segments;

                triangles->emplace_back(vertices[a]);
                triangles->emplace_back(vertices[a1]);
                triangles->emplace_back(vertices[b1]);

                triangles->emplace_back(vertices[a]);
                triangles->emplace_back(vertices[b]);
                triangles->emplace_back(vertices[b1]);
            }
        }

        for (uint32_t i = 0; i < segments; ++i)
        {
            uint32_t const a = i + segments * (segments - 2) + 1;
            uint32_t const b = (i + 1) % segments + segments * (segments - 2) + 1;

            triangles->emplace_back(vertices[vertexCount-1]);
            triangles->emplace_back(vertices[a]);
            triangles->emplace_back(vertices[b]);
        }

        mesh->Init(triangles->data(), triangles->size());

        delete triangles;
        delete[] vertices;

        return mesh;
    }

}