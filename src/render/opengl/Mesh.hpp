#pragma once

#include <cstdint>
#include "render/Mesh.hpp"

namespace Render {
namespace OpenGL {

    typedef uint32_t GLuint;

    class Mesh : Render::Mesh {
        Mesh();
        virtual ~Mesh() override;

        void Init(Vertex *vertices, int vCount);
        //void setupFVF(GAPI::Vertex *v) const;

        void Bind() const;
    private:
        GLuint VAO_ID;
        GLuint VBO_ID;
    };

}
}


