#pragma once

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            class PreprocessorContext final
            {
            public:
                PreprocessorContext();

                void Parse(const U8String& source);
            };
        }
    }
}