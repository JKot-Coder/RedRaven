#pragma once

namespace OpenDemo
{
    namespace Rendering
    {
        class Mesh;

        class Primitives
        {
        public:
            static std::shared_ptr<Mesh> GetSphereMesh(unsigned int segments);
            static std::shared_ptr<Mesh> GetFullScreenQuad();
        };
    }
}