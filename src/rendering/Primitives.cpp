#include "Primitives.hpp"

#include <vector>
#include <array>

#include "common/VecMath.h"

#include "rendering/Render.hpp"
#include "rendering/Mesh.hpp"

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
            float const polar = PI * float(j+1) / float(segments);
            float const sp = (float) sin(polar);
			float const cp = (float) cos(polar);

            for (uint32_t i = 0; i < segments; ++i)
            {
                float const azimuth = 2.0f * PI * float(i) / float(segments);
				float const sa = (float) sin(azimuth);
				float const ca = (float) cos(azimuth);
                float const x = sp * ca;
                float const y = cp;
                float const z = sp * sa;

                vertices[index].position = vec3(x, y, z);
                vertices[index].normal   = vec3(x, y, z);
                index++;
            }
        }

        vertices[index].position = vec3(0.0f, -1.0f, 0.0f);
        vertices[index].normal   = vec3(0.0f, -1.0f, 0.0f);

        std::vector<int32_t> *indexes = new std::vector<int32_t>();

        for (uint32_t i = 0; i < segments; ++i)
        {
            uint32_t const a = i + 1;
            uint32_t const b = (i + 1) % segments + 1;

            indexes->emplace_back(0);
            indexes->emplace_back(b);
            indexes->emplace_back(a);
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

                indexes->emplace_back(a);
                indexes->emplace_back(a1);
                indexes->emplace_back(b1);

                indexes->emplace_back(a);
                indexes->emplace_back(b1);
                indexes->emplace_back(b);
            }
        }

        for (uint32_t i = 0; i < segments; ++i)
        {
            uint32_t const a = i + segments * (segments - 2) + 1;
            uint32_t const b = (i + 1) % segments + segments * (segments - 2) + 1;

            indexes->emplace_back(vertexCount-1);
            indexes->emplace_back(a);
            indexes->emplace_back(b);
        }

        mesh->Init(vertices, vertexCount, indexes->data(), static_cast<int32_t>(indexes->size()));

        delete indexes;
        delete[] vertices;

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::GetFullScreenQuad() {
        static std::shared_ptr<Mesh> fullScreenQuad;

        if (fullScreenQuad == nullptr) {
            const auto &render = Rendering::Instance();
            fullScreenQuad = render->CreateMesh();

            Vertex vertices[6];

            vertices[0].position = vec3( 1.0f, -1.0f, 0.0f);
            vertices[1].position = vec3(-1.0f,  1.0f, 0.0f);
            vertices[2].position = vec3(-1.0f, -1.0f, 0.0f);

            vertices[3].position = vec3( 1.0f, -1.0f, 0.0f);
            vertices[4].position = vec3( 1.0f,  1.0f, 0.0f);
            vertices[5].position = vec3(-1.0f,  1.0f, 0.0f);

            vertices[0].texCoord = vec2( 1.0f, 0.0f);
            vertices[1].texCoord = vec2( 0.0f, 1.0f);
            vertices[2].texCoord = vec2( 0.0f, 0.0f);

            vertices[3].texCoord = vec2( 1.0f, 0.0f);
            vertices[4].texCoord = vec2( 1.0f, 1.0f);
            vertices[5].texCoord = vec2( 0.0f, 1.0f);

            fullScreenQuad->Init(vertices, 6);
        }

        return fullScreenQuad;
    }

}