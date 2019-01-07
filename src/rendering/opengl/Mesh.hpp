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

        virtual void Init(const Vertex *vertices, int vCount) override;
        virtual void Init(const Vertex *vertices, int vCount, const int32_t *indexes, int iCount) override;

        virtual void Init(const std::vector<Vertex> &vertices) override;
        virtual void Init(const std::vector<Vertex> &vertices, const std::vector<int32_t> &indexes) override;

        virtual void Bind() const override;
        virtual void Draw() const override;

    private:
        GLuint vaoId;
        GLuint vboId[2];

        int32_t vCount;
        int32_t iCount;

        void SetupAttributes();
    };

}
}


