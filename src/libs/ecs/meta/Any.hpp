#pragma once

#include "ComponentTraits.hpp"

namespace RR::Ecs::Meta
{
    struct ElementsSpan;

    struct Any
    {
        Any() = delete;
        Any(void* data, const ComponentInfo& componentInfo) : data(data), componentInfo(&componentInfo) { ASSERT(data); }
        template <typename T>
        T& Cast()
        {
            ASSERT(componentInfo->id == Meta::GetComponentId<T>);
            return *reinterpret_cast<T*>(data);
        }

        ElementsSpan Elements() const;
        ComponentId GetComponentId() const;
        std::string_view GetComponentName() const;
        const ComponentInfo* GetComponentInfo() const { return componentInfo; }

    private:
        void* data;
        const ComponentInfo* componentInfo;
    };

    inline Any ComponentInfo::Get(void* data) const { return Any(data, *this); }
}