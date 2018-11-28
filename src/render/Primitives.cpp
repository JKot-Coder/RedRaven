#include "common/VecMath.h"

#include "render/Rendering.hpp"
#include "render/Mesh.hpp"

#include "render/Primitives.hpp"

using namespace Common;

namespace Render {

    std::shared_ptr<Mesh> Primitives::GetSphereMesh(int segments)
    {
        const auto& render = Render::Instance();
        const auto& mesh = render->CreateMesh();

//        Render::Vertex* vertices = new Vertex[segments * segments];
//
//        for (int i = 0; i < segments; i++){
//            float lng = PI * (i + 1) / static_cast<float>(segments);
//
//            for (int j = 0; j < segments; j++) {
//                float lat  = 2.0f * PI * j / static_cast<float>(segments);
//                vertices->position = Common::vec3(lng, lat);
//            }
//        }


//        Render::Vertex* vertices = new Vertex[3];
//
//        vertices[0].position = vec3( -1.0f, -1.0f, -2.0f );
//        vertices[1].position = vec3(  0.0f,  1.0f, -2.0f );
//        vertices[2].position = vec3(  1.0f, -1.0f, -2.0f );

        //TODO: TEMPORARY SOLUTION
        Render::Vertex* vertices = new Vertex[3];

        vertices[0].position = vec3(  0.0f,  0.0f, 0.0f );
        vertices[1].position = vec3(  0.5f,  0.0f, 0.0f );
        vertices[2].position = vec3(  0.5f,  0.5f, 0.0f );

        (void) segments;
        mesh->Init(vertices, 3);
        return mesh;
    }

}