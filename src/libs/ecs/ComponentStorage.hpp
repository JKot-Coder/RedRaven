#pragma once

#include "ecs/ComponentTraits.hpp"
#include "ska/flat_hash_map.h"

namespace RR::Ecs
{
// TODO why we need this
    class ComponentStorage
    {
    public:
        template <typename T>
        ComponentId Register()
        {
            // TODO assert if type registered but with different size.
            constexpr auto componentInfo = ComponentInfo::Create<T>();
            componentsInfo.emplace(componentInfo.id, componentInfo);
            return componentInfo.id;
        }

        ComponentInfo& operator[](ComponentId id) { return componentsInfo[id]; }
        auto find(ComponentId id) { return componentsInfo.find(id); }
        auto find(ComponentId id) const { return componentsInfo.find(id); }
        auto begin() const { return componentsInfo.begin(); }
        auto end() const { return componentsInfo.end(); }

    private:
        ska::flat_hash_map<ComponentId, ComponentInfo> componentsInfo;
    };
}