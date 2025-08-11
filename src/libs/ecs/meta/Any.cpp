#include "Any.hpp"

#include "ComponentTraits.hpp"

namespace RR::Ecs::Meta
{
    PropertiesSpan Any::Properties() const
    {
        return PropertiesSpan(PropertyIterator(data, componentInfo->properties.begin()), PropertyIterator(data, componentInfo->properties.end()));
    }
}