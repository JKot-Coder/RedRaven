#include "vector"

#include "rendering/Render.hpp"
#include "glad/glad.h"

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

    void Mesh::Init(const Rendering::Vertex *vertices, int32_t vCount_) {
        vCount = vCount_;
        iCount = 0;

        glBindVertexArray(vaoId);

        glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        SetupAttributes();

        glBindVertexArray(0);
    }

    void Mesh::Init(const Vertex *vertices, int32_t vCount_, const int32_t *indexes, int32_t iCount_) {
        vCount = vCount_;
        iCount = iCount_;

        glBindVertexArray(vaoId);

        glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(int32_t), indexes, GL_STATIC_DRAW);

        SetupAttributes();

        glBindVertexArray(0);
    }

    void Mesh::Init(const std::vector<Vertex> &vertices) {
        Init(vertices.data(), static_cast<int32_t>(vertices.size()));
    }

    void Mesh::Init(const std::vector<Vertex> &vertices, const std::vector<int32_t> &indexes) {
        Init(vertices.data(), static_cast<int32_t>(vertices.size()), indexes.data(), static_cast<int32_t>(indexes.size()));
    }

    void Mesh::SetupAttributes() {
        glEnableVertexAttribArray(Attributes::POSITION);
        glEnableVertexAttribArray(Attributes::TEXCOORD);
        glEnableVertexAttribArray(Attributes::NORMAL);
        glEnableVertexAttribArray(Attributes::TANGENT);
        glEnableVertexAttribArray(Attributes::BINORMAL);
        glEnableVertexAttribArray(Attributes::COLOR);

        glVertexAttribPointer(Attributes::POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glVertexAttribPointer(Attributes::TEXCOORD, 2, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        glVertexAttribPointer(Attributes::NORMAL,   3, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(Attributes::TANGENT,  3, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        glVertexAttribPointer(Attributes::BINORMAL, 3, GL_FLOAT, true,  sizeof(Vertex), (void*)offsetof(Vertex, binormal));
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