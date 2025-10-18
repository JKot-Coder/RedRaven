#pragma once

#include "math/VectorMath.hpp"

#include "gapi/VertexLayout.hpp"

namespace RR::Render
{
    struct Vertex
    {
        Vector4 pos;
        Vector4 color;

        static const GAPI::VertexLayout& GetVertexLayout()
        {
            static GAPI::VertexLayout vertexLayout = GAPI::VertexLayout::Build()
                                                         .Add("POSITION", 0, GAPI::VertexAttributeType::Float, 4, 0)
                                                         .Add("COLOR", 0, GAPI::VertexAttributeType::Float, 4, 0)
                                                         .Commit();
            return vertexLayout;
        }
    };
}
