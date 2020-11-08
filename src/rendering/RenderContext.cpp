#include "RenderCommandContext.hpp"

#include "rendering/Render.hpp"

namespace OpenDemo
{
    namespace Rendering
    {

        RenderCommandContext::RenderCommandContext()
            : _depthWrite(true)
            , _depthTestFunction(DepthTestFunction::LEQUAL)
            , _renderQuery(new RenderQuery())
        {
        }

    }
}