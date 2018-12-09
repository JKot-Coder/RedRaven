#include "rendering/Render.hpp"
#include "dependencies/glad/glad.h"

#include "Mesh.hpp"

namespace Rendering {
namespace OpenGL {

    Mesh::Mesh() {
        glGenBuffers(1, &VBO_ID);
        glGenVertexArrays(1, &VAO_ID);
    }

    Mesh::~Mesh() {
        glDeleteBuffers(1, &VBO_ID);
        glDeleteVertexArrays(1, &VAO_ID);
    }

    void Mesh::Init(Rendering::Vertex *vertices, int vCount) {
        this->vCount = vCount;

        glBindVertexArray(VAO_ID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(Attributes::POSITION);
        glEnableVertexAttribArray(Attributes::NORMAL);
        glEnableVertexAttribArray(Attributes::TEXCOORD);
        glEnableVertexAttribArray(Attributes::COLOR);

        glVertexAttribPointer(Attributes::POSITION, 3, GL_FLOAT, false, sizeof(*vertices), (void*)offsetof(Vertex, position));
        glVertexAttribPointer(Attributes::NORMAL,   3, GL_FLOAT, true,  sizeof(*vertices), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(Attributes::TEXCOORD, 2, GL_FLOAT, true,  sizeof(*vertices), (void*)offsetof(Vertex, texCoord));
        glVertexAttribPointer(Attributes::COLOR,    4, GL_FLOAT, true,  sizeof(*vertices), (void*)offsetof(Vertex, color));
        glVertexAttribPointer(Attributes::COLOR,    4, GL_FLOAT, true,  sizeof(*vertices), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void Mesh::Bind() const {
        glBindVertexArray(VAO_ID);
    }

    void Mesh::Draw() const {
        Bind();
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, vCount);
        glBindVertexArray(0);
    }

}
}