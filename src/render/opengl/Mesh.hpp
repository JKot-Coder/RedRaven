#pragma once

#include <cstdint>
#include "render/Mesh.hpp"

namespace Render {
namespace OpenGL {

    typedef uint32_t GLuint;

    class Mesh : public Render::Mesh {
    public:
        Mesh();
        virtual ~Mesh() override;

        virtual void Init(Vertex *vertices, int vCount) override;

        virtual void Bind() const override;
        virtual void Draw() const override;
    private:
        GLuint VAO_ID;
        GLuint VBO_ID;

        int32_t vCount;
    };

}
}


