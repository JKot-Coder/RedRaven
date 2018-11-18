#pragma once

namespace Common{
    class Stream;
}

namespace Render
{
    class Shader {
    public:
        virtual ~Shader() {};

        virtual bool LinkSource(Common::Stream* stream) = 0;
    };
}

