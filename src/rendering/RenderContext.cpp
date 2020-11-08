#include "RenderContext.hpp"

#include "rendering/Render.hpp"

namespace OpenDemo
{
    namespace Rendering
    {

        RenderContext::RenderContext()
            : _depthWrite(true)
            , _depthTestFunction(DepthTestFunction::LEQUAL)
            , _renderQuery(new RenderQuery())
        {
        }

    }
}