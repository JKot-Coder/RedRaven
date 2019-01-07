#include "vector"

#include "rendering/Render.hpp"
#include "dependencies/glad/glad.h"

#include "Mesh.hpp"

namespace Rendering {
namespace OpenGL {

    Mesh::Mesh() {
        glGenBuffers(2, vboId);
        glGenVertexArrays(1, &vaoId);
    }

    Mesh::~Mesh() {
        glDeleteBuffers(2, vboId);
        glDeleteVertexArrays(1, &vaoId);
    }

    void Mesh::Init(const Rendering::Vertex *vertices, int vCount) {
        this->vCount = vCount;
        this->iCount = 0;

        glBindVertexArray(vaoId);

        glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        SetupAttributes();

        glBindVertexArray(0);
    }

    void Mesh::Init(const Vertex *vertices, int vCount, const int32_t *indexes, int iCount) {
        this->vCount = vCount;
        this->iCount = iCount;

        glBindVertexArray(vaoId);

        glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(int32_t), indexes, GL_STATIC_DRAW);

        SetupAttributes();

        glBindVertexArray(0);
    }

    void Mesh::Init(const std::vector<Vertex> &vertices) {
        Init(vertices.data(), vertices.size());
    }

    void Mesh::Init(const std::vector<Vertex> &vertices, const std::vector<int32_t> &indexes) {
        Init(vertices.data(), vertices.size(), indexes.data(), indexes.size());
    }

    void Mesh::SetupAttributes() {
        glEnableVertexAttribArray(Attributes::POSITION);
        glEnableVertexAttribArray(Attributes::NORMAL);
        glEnableVertexAttribArray(Attributes::TEXCOORD);
        glEnableVertexAttribArray(Attributes::COLOR);

        glVertexAttribPointer(Attributes::POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glVertexAttribPointer(Attributes::NORMAL,   3, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(Attributes::TEXCOORD, 2, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        glVertexAttribPointer(Attributes::COLOR,    4, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, color));
        glVertexAttribPointer(Attributes::COLOR,    4, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, color));
    };

    void Mesh::Bind() const {
        glBindVertexArray(vaoId);
    }

    void Mesh::Draw() const {
        Bind();

        if(iCount == 0){
            glDrawArrays(GL_TRIANGLES, 0, vCount);
        } else {
            glDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }



}
}