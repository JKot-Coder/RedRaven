#pragma once

#include "ComponentTraits.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Ecs
{
    class ComponentStorage
    {
    public:
        template <typename T>
        ComponentId Register()
        {
            constexpr auto componentInfo = ComponentInfo::Create<T>();
            ASSERT_MSG(isValid(componentInfo), "Component differs from previous registration!");
            componentsInfo.emplace(componentInfo.id, componentInfo);
            return componentInfo.id;
        }

        ComponentInfo& operator[](ComponentId id) { return componentsInfo[id]; }
        auto find(ComponentId id) { return componentsInfo.find(id); }
        auto find(ComponentId id) const { return componentsInfo.find(id); }
        auto begin() const { return componentsInfo.begin(); }
        auto end() const { return componentsInfo.end(); }

    private:
        bool isValid(const ComponentInfo& componentInfo)
        {
            auto it = componentsInfo.find(componentInfo.id);
            if (it == componentsInfo.end())
                return true;

            return it->second == componentInfo;
        }
    private:
        absl::flat_hash_map<ComponentId, ComponentInfo, Ecs::DummyHasher<ComponentId>> componentsInfo;
    };
}