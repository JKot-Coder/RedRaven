#include "Any.hpp"

#include "ElementsIterator.hpp"
#include "ComponentTraits.hpp"

namespace RR::Ecs::Meta
{
    ComponentId Any::GetComponentId() const
    {
        return componentInfo->id;
    }

    std::string_view Any::GetComponentName() const
    {
        return componentInfo->name;
    }

    ElementsSpan Any::Elements() const
    {
        return ElementsSpan(ElementIterator(data, componentInfo->elements.begin()), ElementIterator(data, componentInfo->elements.end()));
    }
}