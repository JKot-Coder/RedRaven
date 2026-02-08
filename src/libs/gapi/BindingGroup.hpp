#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/Resource.hpp"

namespace RR::GAPI
{
    struct BindingElement
    {
        uint32_t bindingIndex;
        const GpuResourceView* view = nullptr;
    };

    struct BindingGroupDesc
    {
        eastl::span<BindingElement> elements;
    };

    class IBindingGroup
    {
    public:
        virtual ~IBindingGroup() = default;
    };

    class BindingGroup final : public Resource<IBindingGroup, false>
    {
    public:
        BindingGroup() : Resource(Type::BindingSet) { }
    };
}