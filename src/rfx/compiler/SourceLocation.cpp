#include "SourceLocation.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            void SourceFile::SetContents(const U8String& content)
            {
                contentSize_ = content.length();                
                content_ = UnownedStringSlice(content);
            }
        }
    }
}