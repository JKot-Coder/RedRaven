#pragma once

#include <memory>
#include <vector>

#include "common/VecMath.h"

using namespace Common;

namespace Rendering {

    class Mesh;
    class Material;
    class Camera;

    struct RenderElement {
    public:
        mat4 modelMatrix;
        std::shared_ptr<Rendering::Material> material;
        std::shared_ptr<Rendering::Mesh> mesh;
    };

    typedef std::vector<RenderElement> RenderQuery;

    class RenderContext {
    public:
        RenderContext() : renderQuery(new RenderQuery()) {}

        inline void SetCamera(std::shared_ptr<Camera> camera) { this->camera = camera; };

        inline std::shared_ptr<Camera> GetCamera() const { return camera; }
        inline RenderQuery& GetRenderQuery() const { return *renderQuery; }

    private:
        std::unique_ptr<RenderQuery> renderQuery;
        std::shared_ptr<Camera> camera;
    };

}