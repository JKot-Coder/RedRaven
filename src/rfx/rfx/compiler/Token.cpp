#include "Token.hpp"

namespace RR
{
    namespace Rfx
    {
        U8String TokenTypeToString(Token::Type type)
        {
            switch (type)
            {
                default:
                    ASSERT(!"unexpected");
                    return "<uknown>";

                // clang-format off
                #define TOKEN(NAME, DESC) \
                    case Token::Type::NAME: \
                        return DESC;
                #include "TokenDefinitions.hpp"
                // clang-format on
            }
        }
    }
}