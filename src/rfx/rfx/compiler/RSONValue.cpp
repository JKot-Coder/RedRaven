#include "RSONValue.hpp"

namespace RR::Rfx
{
    std::string RSONValueTypeToString(RSONValue::Type type)
    {
        static_assert(int(RSONValue::Type::CountOf) == 9);
        switch (type)
        {
            case RSONValue::Type::Invalid: return "Invalid";

            case RSONValue::Type::Bool: return "Bool";
            case RSONValue::Type::Float: return "Float";
            case RSONValue::Type::Integer: return "Integer";
            case RSONValue::Type::Null: return "Null";
            case RSONValue::Type::String: return "String";
            case RSONValue::Type::Reference: return "Reference";

            case RSONValue::Type::Array: return "Array";
            case RSONValue::Type::Object: return "Object";

            default:
                ASSERT(!"unexpected");
                return "<uknown>";
        }
    }
}