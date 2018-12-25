#include "rendering/Render.hpp"

#include "rendering/RenderContext.hpp"

namespace Rendering {

    RenderContext::RenderContext() :
        depthWrite(true),
        depthTestFunction(DepthTestFunction::LEQUAL),
        renderQuery(new RenderQuery())
    {

    }

}
