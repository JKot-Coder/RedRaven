#pragma once

namespace RR::Ecs::Meta
{
    struct ComponentInfo;
    struct PropertiesSpan;

    struct Any
    {
        Any(void* data, ComponentInfo& componentInfo) : data(data), componentInfo(&componentInfo) { ASSERT(data); }
        template <typename T>
        T& Cast()
        {
            ASSERT(componentInfo->id == GetComponentId<T>);
            return *reinterpret_cast<T*>(data);
        }

        PropertiesSpan Properties() const;

    private:
        void* data;
        ComponentInfo* componentInfo;
    };
}