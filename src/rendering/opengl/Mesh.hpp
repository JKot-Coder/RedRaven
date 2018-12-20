#pragma once

#include <cstdint>
#include "rendering/Mesh.hpp"

namespace Rendering {
namespace OpenGL {

    typedef uint32_t GLuint;

    class Mesh final : public Rendering::Mesh {
    public:
        Mesh();
        virtual ~Mesh() override;

        virtual void Init(Vertex *vertices, int vCount) override;

        virtual void Bind() const override;
        virtual void Draw() const override;
    private:
        GLuint vaoId;
        GLuint vboId;

        int32_t vCount;
    };

}
}


