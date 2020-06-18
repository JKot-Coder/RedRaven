#pragma once

namespace OpenDemo
{
    namespace Rendering
    {

        enum DepthTestFunction : int
        {
            NEVER,
            LESS,
            EQUAL,
            LEQUAL,
            GREATER,
            NOTEQUAL,
            GEQUAL,
            ALWAYS,
            DEPTH_TEST_FUNC_MAX
        };

    }
}