#include <render/Rendering.hpp>
#include "dependencies/glad/glad.h"

#include "Mesh.hpp"

namespace Render {
namespace OpenGL {

    Mesh::Mesh() {
        glGenBuffers(1, &VBO_ID);
        glGenVertexArrays(1, &VAO_ID);
    }

    Mesh::~Mesh() {
        glDeleteBuffers(1, &VBO_ID);
        glDeleteVertexArrays(1, &VBO_ID);
    }

    void Mesh::Init(Render::Vertex *vertices, int vCount) {
        glBindVertexArray(VAO_ID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
        glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(Attributes::POSITION);
        glEnableVertexAttribArray(Attributes::NORMAL);
        glEnableVertexAttribArray(Attributes::TEXCOORD);
        glEnableVertexAttribArray(Attributes::COLOR);

        glVertexAttribPointer(Attributes::POSITION, 3, GL_FLOAT, false, sizeof(*vertices), &vertices->position);
        glVertexAttribPointer(Attributes::NORMAL,   3, GL_FLOAT, true,  sizeof(*vertices), &vertices->normal);
        glVertexAttribPointer(Attributes::TEXCOORD, 2, GL_FLOAT, true,  sizeof(*vertices), &vertices->texCoord);
        glVertexAttribPointer(Attributes::COLOR,    4, GL_FLOAT, true,  sizeof(*vertices), &vertices->color);

        glBindVertexArray(0);
    }

}
}