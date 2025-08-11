#include "Any.hpp"

#include "ComponentTraits.hpp"

namespace RR::Ecs::Meta
{
    ElementsSpan Any::Elements() const
    {
        return ElementsSpan(ElementIterator(data, componentInfo->elements.begin()), ElementIterator(data, componentInfo->elements.end()));
    }
}