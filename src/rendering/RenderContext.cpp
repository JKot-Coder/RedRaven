#include "RenderContext.hpp"

#include "rendering/Render.hpp"

namespace Rendering {

    RenderContext::RenderContext() :
        depthWrite(true),
        depthTestFunction(DepthTestFunction::LEQUAL),
        renderQuery(new RenderQuery())
    {

    }

}
